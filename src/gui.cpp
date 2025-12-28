// Copyright [2025] <JiJun Lu, Linru Zhou>
#include "gui.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cstring>

// Windows API for file dialogs
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#endif

// ImGui and GLFW
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "CBackup.h"
#include "CConfig.h"
#include "PackFactory.h"
#include "CompressFactory.h"
#include "EncryptFactory.h"
#include "CBackupRecorder.h"

namespace fs = std::filesystem;

#ifdef _WIN32
/**
 * @brief 文件夹选择对话框的回调函数，用于设置初始路径
 */
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED && lpData) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

/**
 * @brief 打开文件夹选择对话框
 * @param initialPath 初始路径
 * @return 选择的文件夹路径，如果取消则返回空字符串
 */
static std::string BrowseForFolder(const std::string& initialPath = "") {
    BROWSEINFO bi = {0};
    bi.lpszTitle = "Select a folder:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    // 设置初始路径
    static char initialDir[MAX_PATH] = {0};
    if (!initialPath.empty() && fs::exists(initialPath)) {
        strncpy(initialDir, initialPath.c_str(), MAX_PATH - 1);
        initialDir[MAX_PATH - 1] = '\0';
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = reinterpret_cast<LPARAM>(initialDir);
    }
    
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != nullptr) {
        char path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            IMalloc* imalloc = nullptr;
            if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                imalloc->Free(pidl);
                imalloc->Release();
            }
            return std::string(path);
        }
        IMalloc* imalloc = nullptr;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
    return "";
}

/**
 * @brief 打开文件选择对话框
 * @param initialPath 初始路径（可以是文件或文件夹）
 * @param filter 文件过滤器，格式："Description\0*.ext\0"
 * @return 选择的文件路径，如果取消则返回空字符串
 */
static std::string BrowseForFile(const std::string& initialPath = "", const char* filter = "All Files\0*.*\0") {
    OPENFILENAMEA ofn = {0};
    char szFile[260] = {0};
    
    // 如果初始路径是文件，使用其父目录；如果是目录，直接使用
    std::string initialDir = initialPath;
    if (!initialPath.empty() && fs::exists(initialPath)) {
        if (fs::is_regular_file(initialPath)) {
            initialDir = fs::path(initialPath).parent_path().string();
            // 同时设置默认文件名
            strncpy(szFile, initialPath.c_str(), sizeof(szFile) - 1);
        }
    }
    
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDir.empty() ? nullptr : initialDir.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
    return "";
}
#endif

// ============================================================================
// GUI 状态结构
// ============================================================================

/**
 * @brief 备份标签页的状态结构
 * 对应命令行参数：--mode backup --src <path> --dst <relative_path> 
 *                [--include <regex> --pack <packType> --compress <compressType> 
 *                 --encrypt <encryptType> --key <encryptKey> --desc <description>]
 */
struct BackupState {
    char sourcePath[512] = "";           // 源路径（对应 --src）
    char destPath[512] = "";             // 目标相对路径（对应 --dst）
    int sourceType = 0;                   // 0: 文件夹, 1: 文件
    bool enablePack = false;             // 是否启用打包（对应 --pack）
    int packTypeIndex = 0;               // 打包类型索引
    bool enableCompress = false;         // 是否启用压缩（对应 --compress）
    int compressTypeIndex = 0;           // 压缩类型索引
    bool enableEncrypt = false;          // 是否启用加密（对应 --encrypt）
    int encryptTypeIndex = 0;            // 加密类型索引
    char encryptKey[256] = "";           // 加密密钥（对应 --key）
    char includeRegex[256] = "";         // 文件过滤正则表达式（对应 --include）
    char description[512] = "";          // 备份描述（对应 --desc）
    std::string statusMessage = "";      // 状态消息
    bool statusIsError = false;          // 是否为错误消息
};

/**
 * @brief 恢复标签页的状态结构
 * 对应命令行参数：--mode recover --fn <filename> --to <target_path>
 */
struct RecoverState {
    int selectedRecordIndex = -1;       // 选中的备份记录索引（对应 --fn）
    char restoreToPath[512] = "";        // 恢复目标路径（对应 --to）
    char passwordInput[256] = "";        // 密码输入（用于加密备份）
    bool showPasswordDialog = false;     // 是否显示密码对话框
    std::string statusMessage = "";      // 状态消息
    bool statusIsError = false;          // 是否为错误消息
    // 查询相关字段
    int queryType = 0;                   // 0: 按名称, 1: 按时间
    char queryNameInput[256] = "";       // 查询文件名
    char queryStartTime[64] = "";        // 查询开始时间
    char queryEndTime[64] = "";          // 查询结束时间
    std::vector<BackupEntry> queryResults; // 查询结果
    bool isQueryMode = false;            // 是否处于查询模式
    std::string queryStatusMessage = ""; // 查询状态消息
};

/**
 * @brief 备份记录标签页的状态结构
 */
struct RecordsState {
    int selectedRecordIndex = -1;        // 选中的记录索引
    // 查询相关字段
    int queryType = 0;                   // 0: 按名称, 1: 按时间
    char queryNameInput[256] = "";       // 查询文件名
    char queryStartTime[64] = "";        // 查询开始时间
    char queryEndTime[64] = "";          // 查询结束时间
    std::vector<BackupEntry> queryResults; // 查询结果
    bool isQueryMode = false;            // 是否处于查询模式
    std::string queryStatusMessage = ""; // 查询状态消息
};

// ============================================================================
// 业务逻辑函数
// ============================================================================

/**
 * @brief 执行备份操作
 * 
 * 根据 BackupState 中的配置执行备份，对应命令行：
 * --mode backup --src <path> --dst <relative_path> [其他选项]
 * 
 * @param state 备份状态配置
 * @param recorder 备份记录器
 */
static void executeBackup(BackupState& state, CBackupRecorder& recorder) {
    if (strlen(state.sourcePath) == 0 || strlen(state.destPath) == 0) {
        state.statusMessage = "Error: Source path and destination path are required!";
        state.statusIsError = true;
        return;
    }

    try {
        // 计算实际备份路径
        std::string actualBackupPath = fs::absolute(fs::path(state.destPath)).string();

        // 创建配置
        auto config = std::make_shared<CConfig>();
        config->setSourcePath(fs::absolute(fs::path(state.sourcePath)).string())
              .setDestinationPath(actualBackupPath)
              .setRecursiveSearch(true)
              .setDescription(state.description);  // 设置备份描述（对应 --desc）

        // 获取支持的算法类型
        auto packTypes = PackFactory::getSupportedPackTypes();
        auto compressTypes = CompressFactory::getSupportedCompressTypes();
        auto encryptTypes = EncryptFactory::getSupportedEncryptTypes();

        // 设置打包（对应 --pack）
        if (state.enablePack && state.packTypeIndex < static_cast<int>(packTypes.size())) {
            std::string packType = packTypes[state.packTypeIndex];
            if (!PackFactory::isPackTypeSupported(packType)) {
                state.statusMessage = "Error: Packing algorithm type " + packType + " is not supported.";
                state.statusIsError = true;
                return;
            }
            config->setPackType(packType).setPackingEnabled(true);

            // 设置压缩（对应 --compress）
            if (state.enableCompress && state.compressTypeIndex < static_cast<int>(compressTypes.size())) {
                std::string compressType = compressTypes[state.compressTypeIndex];
                if (!CompressFactory::isCompressTypeSupported(compressType)) {
                    state.statusMessage = "Error: Compress algorithm type " + compressType + " is not supported.";
                    state.statusIsError = true;
                    return;
                }
                config->setCompressionType(compressType).setCompressionEnabled(true);
            }

            // 设置加密（对应 --encrypt 和 --key）
            if (state.enableEncrypt && state.encryptTypeIndex < static_cast<int>(encryptTypes.size())) {
                std::string encryptType = encryptTypes[state.encryptTypeIndex];
                if (!EncryptFactory::isEncryptTypeSupported(encryptType)) {
                    state.statusMessage = "Error: Encrypt algorithm type " + encryptType + " is not supported.";
                    state.statusIsError = true;
                    return;
                }
                if (strlen(state.encryptKey) == 0) {
                    state.statusMessage = "Error: Encrypt key is required when encryption is enabled.";
                    state.statusIsError = true;
                    return;
                }
                config->setEncryptType(encryptType)
                      .setEncryptionEnabled(true)
                      .setEncryptionKey(state.encryptKey);
            }
        }

        // 设置文件过滤（对应 --include）
        if (strlen(state.includeRegex) > 0) {
            config->addIncludePattern(state.includeRegex);
        }

        // 执行备份
        CBackup backup;
        std::string destPath = backup.doBackup(config);
        if (destPath.empty()) {
            state.statusMessage = "Backup failed!";
            state.statusIsError = true;
            return;
        }

        // 添加备份记录
        recorder.addBackupRecord(config, destPath);
        recorder.saveBackupRecordsToFile(recorder.getRecorderFilePath());

        state.statusMessage = "Backup finished successfully! -> " + destPath;
        state.statusIsError = false;
    } catch (const std::exception& e) {
        state.statusMessage = "Error: " + std::string(e.what());
        state.statusIsError = true;
    }
}

/**
 * @brief 执行恢复操作
 * 
 * 根据 RecoverState 中的配置执行恢复，对应命令行：
 * --mode recover --fn <filename> --to <target_path>
 * 
 * @param state 恢复状态配置
 * @param recorder 备份记录器
 */
static void executeRecover(RecoverState& state, CBackupRecorder& recorder) {
    if (state.selectedRecordIndex < 0) {
        state.statusMessage = "Error: Please select a backup record!";
        state.statusIsError = true;
        return;
    }

    BackupEntry entry;
    if (state.isQueryMode) {
        // 查询模式下，从查询结果中获取记录
        if (state.selectedRecordIndex >= static_cast<int>(state.queryResults.size())) {
            state.statusMessage = "Error: Invalid record index!";
            state.statusIsError = true;
            return;
        }
        entry = state.queryResults[state.selectedRecordIndex];
    } else {
        // 正常模式下，从完整记录列表中获取
        auto records = recorder.getBackupRecords();
        if (state.selectedRecordIndex >= static_cast<int>(records.size())) {
            state.statusMessage = "Error: Invalid record index!";
            state.statusIsError = true;
            return;
        }
        entry = records[state.selectedRecordIndex];
    }

    if (strlen(state.restoreToPath) == 0) {
        state.statusMessage = "Error: Restore destination path is required!";
        state.statusIsError = true;
        return;
    }

    try {
        std::string restoreTo = fs::absolute(fs::path(state.restoreToPath)).string();
        CBackup backup;

        // 执行恢复操作
        // 如果备份是加密的，从 GUI 密码输入传递到 doRecovery；否则使用无密码的重载
        bool success;
        if (entry.isEncrypted) {
            success = backup.doRecovery(entry, restoreTo, std::string(state.passwordInput));
        } else {
            success = backup.doRecovery(entry, restoreTo);
        }
        
        if (!success) {
            state.statusMessage = "Recovery failed!";
            state.statusIsError = true;
            return;
        }

        state.statusMessage = "Recovery finished successfully! -> " + restoreTo;
        state.statusIsError = false;
        state.showPasswordDialog = false;
        memset(state.passwordInput, 0, sizeof(state.passwordInput));
    } catch (const std::exception& e) {
        state.statusMessage = "Error: " + std::string(e.what());
        state.statusIsError = true;
    }
}

// ============================================================================
// 界面渲染函数
// ============================================================================

/**
 * @brief 渲染备份标签页
 * 
 * 提供备份配置界面，支持所有备份相关的命令行参数：
 * - --src: 源路径选择（文件或文件夹）
 * - --dst: 目标相对路径
 * - --include: 文件过滤正则表达式
 * - --pack: 打包类型选择
 * - --compress: 压缩类型选择
 * - --encrypt: 加密类型选择
 * - --key: 加密密钥输入
 * - --desc: 备份描述
 */
static void renderBackupTab(BackupState& state, CBackupRecorder& recorder) {
    ImGui::Text("Backup Configuration");
    ImGui::Separator();

    // 源路径选择（对应 --src）
    ImGui::Text("Source Path (File or Directory):");
    
    // 文件/文件夹类型选择
    ImGui::RadioButton("Folder", &state.sourceType, 0);
    ImGui::SameLine();
    ImGui::RadioButton("File", &state.sourceType, 1);
    
    // 路径输入框和浏览按钮
    ImGui::InputText("##source", state.sourcePath, sizeof(state.sourcePath));
    ImGui::SameLine();
    if (ImGui::Button("Browse...##src")) {
#ifdef _WIN32
        std::string currentPath = state.sourcePath;
        std::string selectedPath;
        
        // 根据用户选择打开对应的对话框
        if (state.sourceType == 1) {
            // 选择文件
            selectedPath = BrowseForFile(currentPath);
        } else {
            // 选择文件夹（默认）
            selectedPath = BrowseForFolder(currentPath);
        }
        
        if (!selectedPath.empty()) {
            strncpy(state.sourcePath, selectedPath.c_str(), sizeof(state.sourcePath) - 1);
            state.sourcePath[sizeof(state.sourcePath) - 1] = '\0';
        }
#else
        // 非 Windows 平台，保持原有行为
        ImGui::OpenPopup("Source Path Input");
#endif
    }

    // 目标路径（对应 --dst）
    ImGui::Text("Destination Path:");
    ImGui::InputText("##dest", state.destPath, sizeof(state.destPath));
#ifdef _WIN32
    ImGui::SameLine();
    if (ImGui::Button("Browse...##dest")) {
        std::string currentPath = state.destPath;
        std::string selectedPath = BrowseForFolder(currentPath);
        if (!selectedPath.empty()) {
            strncpy(state.destPath, selectedPath.c_str(), sizeof(state.destPath) - 1);
            state.destPath[sizeof(state.destPath) - 1] = '\0';
        }
    }
#else
    // 非 Windows 平台暂不弹出系统对话框
#endif
    ImGui::TextDisabled("(Absolute path or relative to current directory)");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Options:");

    // 打包选项（对应 --pack）
    ImGui::Checkbox("Enable Packing", &state.enablePack);
    if (state.enablePack) {
        ImGui::Indent();
        auto packTypes = PackFactory::getSupportedPackTypes();
        const char* packItems[10];
        for (size_t i = 0; i < packTypes.size() && i < 10; i++) {
            packItems[i] = packTypes[i].c_str();
        }
        ImGui::Combo("Pack Algorithm", &state.packTypeIndex, packItems, static_cast<int>(packTypes.size()));

        // 压缩选项（对应 --compress，仅在打包时可用）
        ImGui::Checkbox("Enable Compression", &state.enableCompress);
        if (state.enableCompress) {
            ImGui::Indent();
            auto compressTypes = CompressFactory::getSupportedCompressTypes();
            const char* compressItems[10];
            for (size_t i = 0; i < compressTypes.size() && i < 10; i++) {
                compressItems[i] = compressTypes[i].c_str();
            }
            ImGui::Combo("Compress Algorithm", &state.compressTypeIndex, compressItems, static_cast<int>(compressTypes.size()));
            ImGui::Unindent();
        }

        // 加密选项（对应 --encrypt 和 --key，仅在打包时可用）
        ImGui::Checkbox("Enable Encryption", &state.enableEncrypt);
        if (state.enableEncrypt) {
            ImGui::Indent();
            auto encryptTypes = EncryptFactory::getSupportedEncryptTypes();
            const char* encryptItems[10];
            for (size_t i = 0; i < encryptTypes.size() && i < 10; i++) {
                encryptItems[i] = encryptTypes[i].c_str();
            }
            ImGui::Combo("Encrypt Algorithm", &state.encryptTypeIndex, encryptItems, static_cast<int>(encryptTypes.size()));
            ImGui::InputText("Encryption Key", state.encryptKey, sizeof(state.encryptKey), ImGuiInputTextFlags_Password);
            ImGui::Unindent();
        }
        ImGui::Unindent();
    }

    // 文件过滤（对应 --include）
    ImGui::Spacing();
    ImGui::Text("File Filter (Regex, optional):");
    ImGui::InputText("##regex", state.includeRegex, sizeof(state.includeRegex));
    ImGui::TextDisabled("Example: .*\\.txt");

    // 备份描述（对应 --desc）
    ImGui::Spacing();
    ImGui::Text("Description (optional):");
    ImGui::InputText("##desc", state.description, sizeof(state.description));

    ImGui::Spacing();
    ImGui::Separator();

    // 执行备份按钮
    if (ImGui::Button("Start Backup", ImVec2(-1, 0))) {
        executeBackup(state, recorder);
    }

    // 状态消息
    if (!state.statusMessage.empty()) {
        ImGui::Spacing();
        if (state.statusIsError) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::TextWrapped("%s", state.statusMessage.c_str());
        ImGui::PopStyleColor();
    }
}

/**
 * @brief 渲染恢复标签页
 * 
 * 提供恢复配置界面，对应命令行参数：
 * - --mode recover --fn <filename> --to <target_path>
 */
static void renderRecoverTab(RecoverState& state, CBackupRecorder& recorder) {
    ImGui::Text("Recovery Configuration");
    ImGui::Separator();

    auto allRecords = recorder.getBackupRecords();
    
    if (allRecords.empty()) {
        ImGui::TextWrapped("No backup records found. Please create a backup first.");
        return;
    }

    // 查询区域
    ImGui::Text("Search Records:");
    const char* queryTypes[] = { "By Name", "By Time Range" };
    ImGui::Combo("Query Type", &state.queryType, queryTypes, IM_ARRAYSIZE(queryTypes));

    if (state.queryType == 0) {
        // 按名称查询
        ImGui::Text("File Name:");
        ImGui::InputText("##queryName", state.queryNameInput, sizeof(state.queryNameInput));
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            if (strlen(state.queryNameInput) > 0) {
                state.queryResults = recorder.findBackupRecordsByFileName(state.queryNameInput);
                state.isQueryMode = true;
                state.selectedRecordIndex = -1;
                if (state.queryResults.empty()) {
                    state.queryStatusMessage = "No records found matching: " + std::string(state.queryNameInput);
                } else {
                    state.queryStatusMessage = "Found " + std::to_string(state.queryResults.size()) + " record(s)";
                }
            } else {
                state.queryStatusMessage = "Error: Please enter a file name to search";
            }
        }
    } else {
        // 按时间范围查询
        ImGui::Text("Start Time (YYYY-MM-DD HH:MM):");
        ImGui::InputText("##startTime", state.queryStartTime, sizeof(state.queryStartTime));
        ImGui::Text("End Time (YYYY-MM-DD HH:MM):");
        ImGui::InputText("##endTime", state.queryEndTime, sizeof(state.queryEndTime));
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            if (strlen(state.queryStartTime) > 0 && strlen(state.queryEndTime) > 0) {
                state.queryResults = recorder.findBackupRecordsByBackupTime(
                    state.queryStartTime, state.queryEndTime);
                state.isQueryMode = true;
                state.selectedRecordIndex = -1;
                if (state.queryResults.empty()) {
                    state.queryStatusMessage = "No records found in time range: " + 
                        std::string(state.queryStartTime) + " to " + std::string(state.queryEndTime);
                } else {
                    state.queryStatusMessage = "Found " + std::to_string(state.queryResults.size()) + " record(s)";
                }
            } else {
                state.queryStatusMessage = "Error: Please enter both start time and end time";
            }
        }
    }

    // 显示查询状态消息
    if (!state.queryStatusMessage.empty()) {
        ImGui::Spacing();
        if (state.queryStatusMessage.find("Error") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::TextWrapped("%s", state.queryStatusMessage.c_str());
        ImGui::PopStyleColor();
    }

    // 清除查询按钮
    if (state.isQueryMode) {
        ImGui::SameLine();
        if (ImGui::Button("Clear Search")) {
            state.isQueryMode = false;
            state.queryResults.clear();
            state.selectedRecordIndex = -1;
            state.queryStatusMessage = "";
            memset(state.queryNameInput, 0, sizeof(state.queryNameInput));
            memset(state.queryStartTime, 0, sizeof(state.queryStartTime));
            memset(state.queryEndTime, 0, sizeof(state.queryEndTime));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 获取要显示的记录列表
    std::vector<BackupEntry> displayRecords;
    if (state.isQueryMode) {
        displayRecords = state.queryResults;
    } else {
        displayRecords = allRecords;
    }

    if (displayRecords.empty()) { 
        ImGui::TextWrapped("No backup records found.");
        return;
    }

    // 备份记录列表（对应 --fn，通过选择记录实现）
    std::string listTitle = state.isQueryMode ? 
        "Search Results - Select Backup Record (" + std::to_string(displayRecords.size()) + " found):" :
        "Select Backup Record (" + std::to_string(displayRecords.size()) + " total):";
    ImGui::Text("%s", listTitle.c_str());
    if (ImGui::BeginListBox("##records", ImVec2(-1, 200))) {
        for (size_t i = 0; i < displayRecords.size(); i++) {
            const auto& record = displayRecords[i];
            std::string label = record.fileName + " @ " + record.backupTime;
            if (record.isPacked) label += " [Pack]";
            if (record.isCompressed) label += " [Compress]";
            if (record.isEncrypted) label += " [Encrypt]";
            
            if (ImGui::Selectable(label.c_str(), state.selectedRecordIndex == static_cast<int>(i))) {
                state.selectedRecordIndex = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();
    }

    if (state.selectedRecordIndex >= 0 && state.selectedRecordIndex < static_cast<int>(displayRecords.size())) {
        const auto& selected = displayRecords[state.selectedRecordIndex];
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Selected Record Details:");
        ImGui::Text("File Name: %s", selected.fileName.c_str());
        ImGui::Text("Source Path: %s", selected.sourceFullPath.c_str());
        ImGui::Text("Backup Time: %s", selected.backupTime.c_str());
        ImGui::Text("Backup File: %s", selected.backupFileName.c_str());
        ImGui::Text("Packed: %s", selected.isPacked ? "Yes" : "No");
        ImGui::Text("Compressed: %s", selected.isCompressed ? "Yes" : "No");
        ImGui::Text("Encrypted: %s", selected.isEncrypted ? "Yes" : "No");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Restore Destination Path:");  // 对应 --to
        ImGui::InputText("##restoreTo", state.restoreToPath, sizeof(state.restoreToPath));
#ifdef _WIN32
        ImGui::SameLine();
        if (ImGui::Button("Browse...##restore")) {
            std::string selectedPath = BrowseForFolder(state.restoreToPath);
            if (!selectedPath.empty()) {
                strncpy(state.restoreToPath, selectedPath.c_str(), sizeof(state.restoreToPath) - 1);
                state.restoreToPath[sizeof(state.restoreToPath) - 1] = '\0';
            }
        }
#endif

        ImGui::Spacing();
        if (ImGui::Button("Start Recovery", ImVec2(-1, 0))) {
            if (selected.isEncrypted) {
                state.showPasswordDialog = true;
            } else {
                executeRecover(state, recorder);
            }
        }
    }

    // 密码输入对话框
    if (state.showPasswordDialog) {
        ImGui::OpenPopup("Enter Password");
    }

    if (ImGui::BeginPopupModal("Enter Password", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("This backup is encrypted. Please enter the password:");
        ImGui::Spacing();
        ImGui::InputText("Password", state.passwordInput, sizeof(state.passwordInput), ImGuiInputTextFlags_Password);
        
        // 在对话框中显示错误消息
        if (!state.statusMessage.empty() && state.statusIsError) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::TextWrapped("%s", state.statusMessage.c_str());
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (strlen(state.passwordInput) > 0) {
                executeRecover(state, recorder);
                if (!state.statusIsError) {
                    // 成功后才关闭对话框
                    state.showPasswordDialog = false;
                    ImGui::CloseCurrentPopup();
                }
            } else {
                state.statusMessage = "Error: Password cannot be empty!";
                state.statusIsError = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            state.showPasswordDialog = false;
            memset(state.passwordInput, 0, sizeof(state.passwordInput));
            state.statusMessage = "";
            state.statusIsError = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // 状态消息
    if (!state.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        if (state.statusIsError) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::TextWrapped("%s", state.statusMessage.c_str());
        ImGui::PopStyleColor();
    }
}

/**
 * @brief 渲染备份记录查看界面
 */
static void renderRecordsTab(RecordsState& state, CBackupRecorder& recorder) {
    ImGui::Text("Backup Records");
    ImGui::Separator();

    // 刷新按钮
    if (ImGui::Button("Refresh Records")) {
        recorder.loadBackupRecordsFromFile(recorder.getRecorderFilePath());
        state.isQueryMode = false; // 刷新时退出查询模式
        state.queryResults.clear();
        state.selectedRecordIndex = -1;
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 查询区域
    ImGui::Text("Search Records:");
    const char* queryTypes[] = { "By Name", "By Time Range" };
    ImGui::Combo("Query Type", &state.queryType, queryTypes, IM_ARRAYSIZE(queryTypes));

    if (state.queryType == 0) {
        // 按名称查询
        ImGui::Text("File Name:");
        ImGui::InputText("##queryName", state.queryNameInput, sizeof(state.queryNameInput));
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            if (strlen(state.queryNameInput) > 0) {
                state.queryResults = recorder.findBackupRecordsByFileName(state.queryNameInput);
                state.isQueryMode = true;
                state.selectedRecordIndex = -1;
                if (state.queryResults.empty()) {
                    state.queryStatusMessage = "No records found matching: " + std::string(state.queryNameInput);
                } else {
                    state.queryStatusMessage = "Found " + std::to_string(state.queryResults.size()) + " record(s)";
                }
            } else {
                state.queryStatusMessage = "Error: Please enter a file name to search";
            }
        }
    } else {
        // 按时间范围查询
        ImGui::Text("Start Time (YYYY-MM-DD HH:MM):");
        ImGui::InputText("##startTime", state.queryStartTime, sizeof(state.queryStartTime));
        ImGui::Text("End Time (YYYY-MM-DD HH:MM):");
        ImGui::InputText("##endTime", state.queryEndTime, sizeof(state.queryEndTime));
        ImGui::SameLine();
        if (ImGui::Button("Search")) {
            if (strlen(state.queryStartTime) > 0 && strlen(state.queryEndTime) > 0) {
                state.queryResults = recorder.findBackupRecordsByBackupTime(
                    state.queryStartTime, state.queryEndTime);
                state.isQueryMode = true;
                state.selectedRecordIndex = -1;
                if (state.queryResults.empty()) {
                    state.queryStatusMessage = "No records found in time range: " + 
                        std::string(state.queryStartTime) + " to " + std::string(state.queryEndTime);
                } else {
                    state.queryStatusMessage = "Found " + std::to_string(state.queryResults.size()) + " record(s)";
                }
            } else {
                state.queryStatusMessage = "Error: Please enter both start time and end time";
            }
        }
    }

    // 显示查询状态消息
    if (!state.queryStatusMessage.empty()) {
        ImGui::Spacing();
        if (state.queryStatusMessage.find("Error") != std::string::npos) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        ImGui::TextWrapped("%s", state.queryStatusMessage.c_str());
        ImGui::PopStyleColor();
    }

    // 清除查询按钮
    if (state.isQueryMode) {
        ImGui::SameLine();
        if (ImGui::Button("Clear Search")) {
            state.isQueryMode = false;
            state.queryResults.clear();
            state.selectedRecordIndex = -1;
            state.queryStatusMessage = "";
            memset(state.queryNameInput, 0, sizeof(state.queryNameInput));
            memset(state.queryStartTime, 0, sizeof(state.queryStartTime));
            memset(state.queryEndTime, 0, sizeof(state.queryEndTime));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 获取要显示的记录列表
    std::vector<BackupEntry> displayRecords;
    if (state.isQueryMode) {
        displayRecords = state.queryResults;
    } else {
        displayRecords = recorder.getBackupRecords();
    }

    if (displayRecords.empty()) {
        ImGui::TextWrapped("No backup records found.");
        return;
    }

    // 记录列表
    std::string listTitle = state.isQueryMode ? 
        "Search Results (" + std::to_string(displayRecords.size()) + " found):" :
        "Backup Records List (" + std::to_string(displayRecords.size()) + " total):";
    ImGui::Text("%s", listTitle.c_str());
    
    if (ImGui::BeginListBox("##recordsList", ImVec2(-1, 300))) {
        for (size_t i = 0; i < displayRecords.size(); i++) {
            const auto& record = displayRecords[i];
            std::string label = "[" + std::to_string(i) + "] " + record.fileName + " @ " + record.backupTime;
            
            if (ImGui::Selectable(label.c_str(), state.selectedRecordIndex == static_cast<int>(i))) {
                state.selectedRecordIndex = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();
    }

    // 显示选中记录的详情
    if (state.selectedRecordIndex >= 0 && state.selectedRecordIndex < static_cast<int>(displayRecords.size())) {
        const auto& record = displayRecords[state.selectedRecordIndex];
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Record Details:");
        ImGui::Text("File Name: %s", record.fileName.c_str());
        ImGui::Text("Source Full Path: %s", record.sourceFullPath.c_str());
        ImGui::Text("Destination Directory: %s", record.destDirectory.c_str());
        ImGui::Text("Backup File Name: %s", record.backupFileName.c_str());
        ImGui::Text("Backup Time: %s", record.backupTime.c_str());
        ImGui::Text("Is Packed: %s", record.isPacked ? "Yes" : "No");
        ImGui::Text("Is Compressed: %s", record.isCompressed ? "Yes" : "No");
        ImGui::Text("Is Encrypted: %s", record.isEncrypted ? "Yes" : "No");
        if (!record.description.empty()) {
            ImGui::Text("Description: %s", record.description.c_str());
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // 删除按钮
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Delete Record", ImVec2(-1, 0))) {
            // 找到原始记录在完整列表中的索引
            auto allRecords = recorder.getBackupRecords();
            size_t originalIndex = recorder.getBackupRecordIndex(record);
            
            if (originalIndex != std::string::npos) {
                // 删除记录
                if (recorder.deleteBackupRecord(originalIndex)) {
                    // 保存到文件
                    recorder.saveBackupRecordsToFile(recorder.getRecorderFilePath());
                    
                    // 如果在查询模式下，需要更新查询结果
                    if (state.isQueryMode) {
                        // 从查询结果中移除已删除的记录
                        state.queryResults.erase(state.queryResults.begin() + state.selectedRecordIndex);
                        // 如果查询结果为空，退出查询模式
                        if (state.queryResults.empty()) {
                            state.isQueryMode = false;
                            state.queryStatusMessage = "";
                        }
                    }
                    
                    // 清除选中状态
                    state.selectedRecordIndex = -1;
                }
            }
        }
        ImGui::PopStyleColor(3);
    }
}

// ============================================================================
// 主函数
// ============================================================================

/**
 * @brief 运行备份系统图形化界面
 * 
 * 这是 GUI 模块的主入口函数，初始化所有必要的组件并运行主循环。
 * 备份路径完全由用户指定，不依赖任何仓库概念。
 */
int runBackupGUI() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1200, 1000, "Backup System", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 启用垂直同步

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    // 加载更大、更醒目的字体（Windows 平台使用 Segoe UI 系统字体）
#ifdef _WIN32
    // 主字体（较大号）
    ImFont* largeFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 22.0f);
    // 粗体用于强调/标题
    ImFont* largeBoldFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeuib.ttf", 22.0f);
    // 如果可以，使用粗体作为默认字体以使文本更清晰可读（适合老年用户）
    if (largeBoldFont) {
        io.FontDefault = largeBoldFont;
    } else if (largeFont) {
        io.FontDefault = largeFont;
    }
    // 调整样式以适配更大字体：增大控件内边距、项间距及整体缩放
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.25f);            // 统一放大 25%
    style.FramePadding    = ImVec2(10, 8); // 增大输入框/按钮内边距
    style.ItemSpacing     = ImVec2(12, 10);// 增大控件间距
    style.WindowPadding   = ImVec2(14, 12);// 窗口边距
    io.FontGlobalScale = 1.0f;
#else
    // 非 Windows 平台放大整体样式（不强制指定系统字体）
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.25f);
    style.FramePadding    = ImVec2(10, 8);
    style.ItemSpacing     = ImVec2(12, 10);
    style.WindowPadding   = ImVec2(14, 12);
    io.FontGlobalScale = 1.0f;
#endif

    // 设置样式
    ImGui::StyleColorsDark();

    // 初始化 ImGui 平台/渲染器绑定 
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 初始化备份记录器
    // 使用默认构造函数，在当前目录创建 backup_records.json 文件
    CBackupRecorder backupRecorder;
    backupRecorder.loadBackupRecordsFromFile(backupRecorder.getRecorderFilePath());

    // GUI 状态
    BackupState backupState;
    RecoverState recoverState;
    RecordsState recordsState;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 创建主窗口
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Backup System", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

        // 菜单栏
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // 标签页
        if (ImGui::BeginTabBar("MainTabs")) {
            // 备份标签页
            if (ImGui::BeginTabItem("Backup")) {
                renderBackupTab(backupState, backupRecorder);
                ImGui::EndTabItem();
            }

            // 还原标签页
            if (ImGui::BeginTabItem("Recover")) {
                renderRecoverTab(recoverState, backupRecorder);
                ImGui::EndTabItem();
            }

            // 备份记录标签页
            if (ImGui::BeginTabItem("Records")) {
                renderRecordsTab(recordsState, backupRecorder);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        // 渲染
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 清理
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

