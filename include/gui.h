// Copyright [2025] <JiJun Lu, Linru Zhou>
#ifndef INCLUDE_GUI_H_
#define INCLUDE_GUI_H_

#include <string>

/**
 * @brief 备份系统图形化界面模块
 * 
 * 本模块提供了基于 ImGui 和 GLFW 的图形化用户界面，用于备份系统的所有功能。
 * 支持的功能包括：
 * - 备份操作（支持打包、压缩、加密）
 * - 恢复操作（支持加密备份的密码输入）
 * - 备份记录管理（查看、查询、删除）
 * 
 * 所有命令行参数的功能都可以通过图形化界面实现。
 */

/**
 * @brief 运行备份系统图形化界面
 * 
 * 初始化 GLFW 窗口和 ImGui 上下文，创建主窗口并运行事件循环。
 * 提供三个主要标签页：
 * 1. Backup - 备份配置和执行
 * 2. Recover - 恢复配置和执行
 * 3. Records - 备份记录查看和管理
 * 
 * @return int 返回码，0 表示正常退出，非 0 表示错误
 * 
 * @note 此函数会阻塞直到窗口关闭
 * @note 在 Windows 平台下支持文件/文件夹选择对话框
 * @note 备份路径完全由用户指定，不依赖任何仓库概念
 */
int runBackupGUI();

#endif  // INCLUDE_GUI_H_

