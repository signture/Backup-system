#include <gtest/gtest.h>
#include <string>
#include "CBackupRecorder.h"
#include "testUtils.h"

std::string defaultPath = "backup_records.json";

// 载入和保存功能测试
TEST(RecorderTest, LoadAndSaveBackupRecords){  
    // 清理之前的文件
    CleanupTestFile(defaultPath);

    CBackupRecorder recorder;
    std::string testFilePath = "test_backup_records.json";

    // 先在recorder里添加几条备份记录
    BackupEntry entry1("file1.txt", "./file1.txt", "./backup", "backup1", "2023-12-01 12:00:00", false, false, false, "test 1");
    BackupEntry entry2("file2.txt", "./file2.txt", "./backup", "backup2", "2023-12-02 12:00:00", false, false, false, "test 2");
    recorder.addBackupRecord(entry1);
    recorder.addBackupRecord(entry2);

    // 测试保存备份记录
    EXPECT_TRUE(recorder.saveBackupRecordsToFile(testFilePath));

    BackupEntry entry3("file3.txt", "./file3.txt", "./backup", "backup3", "2023-12-03 12:00:00", false, false, false, "test 3");
    BackupEntry entry4("file4.txt", "./file4.txt", "./backup", "backup4", "2023-12-04 12:00:00", false, false, false, "test 4");
    recorder.addBackupRecord(entry3);
    recorder.addBackupRecord(entry4);

    // 测试加载备份记录
    EXPECT_TRUE(recorder.loadBackupRecordsFromFile(testFilePath));

    // 测试是否加载了正确的备份记录
    EXPECT_EQ(recorder.getBackupRecords().size(), 2);
    EXPECT_EQ(recorder.getBackupRecords()[0], entry1);
    EXPECT_EQ(recorder.getBackupRecords()[1], entry2);

    // 清理测试文件
    CleanupTestFile(testFilePath);
}

// 增添功能测试
TEST(RecorderTest, AddBackupRecord){
    // 清理之前的文件
    CleanupTestFile(defaultPath);

    CBackupRecorder recorder;

    BackupEntry entry1("file1.txt", "./file1.txt", "./backup", "backup1", "2023-12-01 12:00:00", false, false, false, "test 1");
    recorder.addBackupRecord(entry1);
    EXPECT_EQ(recorder.getBackupRecords().size(), 1);
    EXPECT_EQ(recorder.getBackupRecords()[0], entry1);

    BackupEntry entry2("file2.txt", "./file2.txt", "./backup", "backup2", "2023-12-02 12:00:00", false, false, false, "test 2");
    recorder.addBackupRecord(entry2);
    EXPECT_EQ(recorder.getBackupRecords().size(), 2);
    EXPECT_EQ(recorder.getBackupRecords()[1], entry2);

}

// 查找功能测试
TEST(RecorderTest, FindBackupRecord){
    // 清理之前的文件
    CleanupTestFile(defaultPath);

    CBackupRecorder recorder;

    BackupEntry entry1("file1.txt", "./file1.txt", "./backup", "backup1", "2023-12-01 12:00:00", false, false, false, "test 1");
    recorder.addBackupRecord(entry1);
    BackupEntry entry2("file2.txt", "./file2.txt", "./backup", "backup2", "2023-12-02 12:00:00", false, false, false, "test 2");
    recorder.addBackupRecord(entry2);
    BackupEntry entry3("file3.txt", "./file3.txt", "./backup", "backup3", "2023-12-03 12:00:00", false, false, false, "test 3");    
    recorder.addBackupRecord(entry3);
    BackupEntry entry4("file1.txt", "./file1.txt", "./backup", "backup4", "2023-12-04 12:00:00", false, false, false, "test 4");
    recorder.addBackupRecord(entry4);

    // 按名查找
    auto file2Results = recorder.findBackupRecordsByFileName("file2.txt");
    EXPECT_EQ(file2Results.size(), 1);
    if (!file2Results.empty()) {
        EXPECT_EQ(file2Results[0], entry2);
    }
    
    auto file1Results = recorder.findBackupRecordsByFileName("file1.txt");
    EXPECT_EQ(file1Results.size(), 2);

    // 通过时间查找
    auto timeResults1 = recorder.findBackupRecordsByBackupTime("2023-12-01 00:00:00", "2023-12-02 23:59:59");
    EXPECT_EQ(timeResults1.size(), 2);
    
    auto timeResults2 = recorder.findBackupRecordsByBackupTime("2023-12-03 00:00:00", "2023-12-04 23:59:59");
    EXPECT_EQ(timeResults2.size(), 2);
    
    auto timeResults3 = recorder.findBackupRecordsByBackupTime("2023-12-01 00:00:00", "2023-12-01 23:59:59");
    EXPECT_EQ(timeResults3.size(), 1);
    if (!timeResults3.empty()) {
        EXPECT_EQ(timeResults3[0], entry1);
    }

}

// 删除功能测试
TEST(RecorderTest, DeleteBackupRecord){
    // 清理之前的文件
    CleanupTestFile(defaultPath);

    CBackupRecorder recorder;

    BackupEntry entry1("file1.txt", "./file1.txt", "./backup", "backup1", "2023-12-01 12:00:00", false, false, false, "test 1");
    recorder.addBackupRecord(entry1);
    BackupEntry entry2("file2.txt", "./file2.txt", "./backup", "backup2", "2023-12-02 12:00:00", false, false, false, "test 2");
    recorder.addBackupRecord(entry2);
    BackupEntry entry3("file3.txt", "./file3.txt", "./backup", "backup3", "2023-12-03 12:00:00", false, false, false, "test 3");    
    recorder.addBackupRecord(entry3);
    BackupEntry entry4("file1.txt", "./file1.txt", "./backup", "backup4", "2023-12-04 12:00:00", false, false, false, "test 4");
    recorder.addBackupRecord(entry4);

    // 删除备份记录
    recorder.deleteBackupRecord(entry1);
    EXPECT_EQ(recorder.getBackupRecords().size(), 3);
    EXPECT_EQ(recorder.findBackupRecordsByFileName("file1.txt").size(), 1);
    EXPECT_EQ(recorder.findBackupRecordsByFileName("file2.txt").size(), 1);

    // 检查是否删除了正确的记录
    EXPECT_EQ(recorder.findBackupRecordsByBackupTime("2023-12-01 00:00:00", "2023-12-01 23:59:59").size(), 0);

}


// 修改功能测试
TEST(RecorderTest, ModifyBackupRecord){// 清理之前的文件
    CleanupTestFile(defaultPath);
    CBackupRecorder recorder;

    BackupEntry entry1("file1.txt", "./file1.txt", "./backup", "backup1", "2023-12-01 12:00:00", false, false, false, "test 1");
    recorder.addBackupRecord(entry1);
    BackupEntry entry2("file2.txt", "./file2.txt", "./backup", "backup2", "2023-12-02 12:00:00", false, false, false, "test 2");
    recorder.addBackupRecord(entry2);
    BackupEntry entry3("file3.txt", "./file3.txt", "./backup", "backup3", "2023-12-03 12:00:00", false, false, false, "test 3");    
    recorder.addBackupRecord(entry3);
    BackupEntry entry4("file1.txt", "./file1.txt", "./backup", "backup4", "2023-12-04 12:00:00", false, false, false, "test 4");
    recorder.addBackupRecord(entry4);

    // 修改备份记录
    BackupEntry modifiedEntry1("file2.txt", "./file2_modified.txt", "./backup", "backup2_modified", "2023-12-01 12:00:00", false, false, false, "");
    recorder.modifyBackupRecord(entry2, modifiedEntry1);
    EXPECT_EQ(recorder.findBackupRecordsByFileName("file2.txt").size(), 1);
    EXPECT_EQ(recorder.findBackupRecordsByFileName("file2.txt")[0], modifiedEntry1);
    EXPECT_EQ(recorder.getBackupRecords().size(), 4);
}