/*
 * Tencent is pleased to support the open source community by making
 * MMKV available.
 *
 * Copyright (C) 2018 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MMKV_MMKV_H
#define MMKV_MMKV_H

#include "MMKVPredef.h"

#include "InterProcessLock.h"
#include "MMBuffer.h"
#include "MMKVMetaInfo.hpp"
#include "MemoryFile.h"
#include "ThreadLock.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace mmkv {
class CodedOutputData;
} // namespace mmkv

MMKV_NAMESPACE_BEGIN

enum MMKVMode : uint32_t {
    MMKV_SINGLE_PROCESS = 0x1,
    MMKV_MULTI_PROCESS = 0x2,
#ifdef MMKV_ANDROID
    CONTEXT_MODE_MULTI_PROCESS = 0x4, // in case someone mistakenly pass Context.MODE_MULTI_PROCESS
    MMKV_ASHMEM = 0x8,
#endif
};

class MMKV {
#ifndef MMKV_ANDROID
    MMKV(const std::string &mmapID, MMKVMode mode, std::string *cryptKey, MMKV_PATH_TYPE *relativePath);
    std::string m_mmapKey;
#else
    MMKV(const std::string &mmapID, int size, MMKVMode mode, std::string *cryptKey, MMKV_PATH_TYPE *relativePath);

    MMKV(const std::string &mmapID, int ashmemFD, int ashmemMetaFd, std::string *cryptKey = nullptr);
#endif

    ~MMKV();

    mmkv::MMKVMap m_dic;
    std::string m_mmapID;
    MMKV_PATH_TYPE m_path;
    MMKV_PATH_TYPE m_crcPath;

    mmkv::MemoryFile m_file;
    size_t m_actualSize;
    mmkv::CodedOutputData *m_output;

    bool m_needLoadFromFile;
    bool m_hasFullWriteback;

    uint32_t m_crcDigest;
    mmkv::MemoryFile m_metaFile;
    mmkv::MMKVMetaInfo m_metaInfo;

    mmkv::AESCrypt *m_crypter;

    mmkv::ThreadLock m_lock;
    mmkv::FileLock m_fileLock;
    mmkv::InterProcessLock m_sharedProcessLock;
    mmkv::InterProcessLock m_exclusiveProcessLock;

#ifdef MMKV_IOS_OR_MAC
    using MMKV_KEY_TYPE = NSString *__unsafe_unretained;
    using MMKV_KEY_CLEAN_TYPE = NSString *;
#    define MMKV_IS_KEY_EMPTY(key) (key.length <= 0)
#else
    using MMKV_KEY_TYPE = const std::string &;
    using MMKV_KEY_CLEAN_TYPE = std::string;
#    define MMKV_IS_KEY_EMPTY(key) key.empty()
#endif

    void loadFromFile();

    void partialLoadFromFile();

    void checkDataValid(bool &loadFromFile, bool &needFullWriteback);

    void checkLoadData();

    bool isFileValid();

    bool checkFileCRCValid(size_t actualSize, uint32_t crcDigest);

    void recaculateCRCDigestWithIV(const void *iv);

    void updateCRCDigest(const uint8_t *ptr, size_t length);

    size_t readActualSize();

    void oldStyleWriteActualSize(size_t actualSize);

    bool writeActualSize(size_t size, uint32_t crcDigest, const void *iv, bool increaseSequence);

    bool ensureMemorySize(size_t newSize);

    bool fullWriteback();

    bool doFullWriteBack(mmkv::MMBuffer &&allData);

    const mmkv::MMBuffer &getDataForKey(MMKV_KEY_TYPE key);

    bool setDataForKey(mmkv::MMBuffer &&data, MMKV_KEY_TYPE key);

    bool removeDataForKey(MMKV_KEY_TYPE key);

    bool appendDataWithKey(const mmkv::MMBuffer &data, MMKV_KEY_TYPE key);

    void notifyContentChanged();

#ifdef MMKV_ANDROID
    void checkReSetCryptKey(int fd, int metaFD, std::string *cryptKey);
#endif

#ifdef MMKV_IOS
    typedef void (^WriteBlock)(mmkv::CodedOutputData *output);
    bool protectFromBackgroundWriting(size_t size, WriteBlock block);
#endif

public:
    // call this before getting any MMKV instance
    static void initializeMMKV(const MMKV_PATH_TYPE &rootDir, MMKVLogLevel logLevel = MMKVLogInfo);

    // a generic purpose instance
    static MMKV *defaultMMKV(MMKVMode mode = MMKV_SINGLE_PROCESS, std::string *cryptKey = nullptr);

#ifndef MMKV_ANDROID
    // mmapID: any unique ID (com.tencent.xin.pay, etc)
    // if you want a per-user mmkv, you could merge user-id within mmapID
    static MMKV *mmkvWithID(const std::string &mmapID,
                            MMKVMode mode = MMKV_SINGLE_PROCESS,
                            std::string *cryptKey = nullptr,
                            MMKV_PATH_TYPE *relativePath = nullptr);
#else
    // mmapID: any unique ID (com.tencent.xin.pay, etc)
    // if you want a per-user mmkv, you could merge user-id within mmapID
    static MMKV *mmkvWithID(const std::string &mmapID,
                            int size = mmkv::DEFAULT_MMAP_SIZE,
                            MMKVMode mode = MMKV_SINGLE_PROCESS,
                            std::string *cryptKey = nullptr,
                            MMKV_PATH_TYPE *relativePath = nullptr);

    static MMKV *mmkvWithAshmemFD(const std::string &mmapID, int fd, int metaFD, std::string *cryptKey = nullptr);

    int ashmemFD() { return (m_file.m_fileType & mmkv::MMFILE_TYPE_ASHMEM) ? m_file.getFd() : -1; }

    int ashmemMetaFD() { return (m_file.m_fileType & mmkv::MMFILE_TYPE_ASHMEM) ? m_metaFile.getFd() : -1; }
#endif // MMKV_ANDROID

    static void onExit();

    const std::string &mmapID();

    const bool m_isInterProcess;

    std::string cryptKey();

    // transform plain text into encrypted text, or vice versa with empty cryptKey
    // you can change existing crypt key with different cryptKey
    bool reKey(const std::string &cryptKey);

    // just reset cryptKey (will not encrypt or decrypt anything)
    // usually you should call this method after other process reKey() the multi-process mmkv
    void checkReSetCryptKey(const std::string *cryptKey);

    bool set(bool value, MMKV_KEY_TYPE key);

    bool set(int32_t value, MMKV_KEY_TYPE key);

    bool set(uint32_t value, MMKV_KEY_TYPE key);

    bool set(int64_t value, MMKV_KEY_TYPE key);

    bool set(uint64_t value, MMKV_KEY_TYPE key);

    bool set(float value, MMKV_KEY_TYPE key);

    bool set(double value, MMKV_KEY_TYPE key);

    // avoid unexpected type conversion (pointer to bool, etc)
    template <typename T>
    bool set(T value, MMKV_KEY_TYPE key) = delete;

#ifdef MMKV_IOS_OR_MAC
    bool set(NSObject<NSCoding> *__unsafe_unretained obj, MMKV_KEY_TYPE key);

    NSObject *getObject(MMKV_KEY_TYPE key, Class cls);
#else
    bool set(const char *value, MMKV_KEY_TYPE key);

    bool set(const std::string &value, MMKV_KEY_TYPE key);

    bool set(const mmkv::MMBuffer &value, MMKV_KEY_TYPE key);

    bool set(const std::vector<std::string> &vector, MMKV_KEY_TYPE key);

    bool getString(MMKV_KEY_TYPE key, std::string &result);

    mmkv::MMBuffer getBytes(MMKV_KEY_TYPE key);

    bool getVector(MMKV_KEY_TYPE key, std::vector<std::string> &result);
#endif // MMKV_IOS_OR_MAC

    bool getBool(MMKV_KEY_TYPE key, bool defaultValue = false);

    int32_t getInt32(MMKV_KEY_TYPE key, int32_t defaultValue = 0);

    uint32_t getUInt32(MMKV_KEY_TYPE key, uint32_t defaultValue = 0);

    int64_t getInt64(MMKV_KEY_TYPE key, int64_t defaultValue = 0);

    uint64_t getUInt64(MMKV_KEY_TYPE key, uint64_t defaultValue = 0);

    float getFloat(MMKV_KEY_TYPE key, float defaultValue = 0);

    double getDouble(MMKV_KEY_TYPE key, double defaultValue = 0);

    size_t getValueSize(MMKV_KEY_TYPE key, bool actualSize);

    int32_t writeValueToBuffer(MMKV_KEY_TYPE key, void *ptr, int32_t size);

    bool containsKey(MMKV_KEY_TYPE key);

    size_t count();

    size_t totalSize();

    size_t actualSize();

#ifdef MMKV_IOS_OR_MAC
    NSArray *allKeys();

    void removeValuesForKeys(NSArray *arrKeys);

    typedef void (^EnumerateBlock)(NSString *key, BOOL *stop);
    void enumerateKeys(EnumerateBlock block);

    static void minimalInit(MMKV_PATH_TYPE defaultRootDir);

#    ifdef MMKV_IOS
    static void setIsInBackground(bool isInBackground);
#    endif
#else
    std::vector<MMKV_KEY_CLEAN_TYPE> allKeys();

    void removeValuesForKeys(const std::vector<MMKV_KEY_CLEAN_TYPE> &arrKeys);
#endif // MMKV_IOS_OR_MAC

    void removeValueForKey(MMKV_KEY_TYPE key);

    void clearAll();

    // MMKV's size won't reduce after deleting key-values
    // call this method after lots of deleting if you care about disk usage
    // note that `clearAll` has the similar effect of `trim`
    void trim();

    // call this method if the instance is no longer needed in the near future
    // any subsequent call to the instance is undefined behavior
    void close();

    // call this method if you are facing memory-warning
    // any subsequent call to the instance will load all key-values from file again
    void clearMemoryCache();

    // you don't need to call this, really, I mean it
    // unless you worry about running out of battery
    void sync(SyncFlag flag = MMKV_SYNC);

    void lock() { m_exclusiveProcessLock.lock(); }
    void unlock() { m_exclusiveProcessLock.unlock(); }
    bool try_lock() { return m_exclusiveProcessLock.try_lock(); }

    // check if content changed by other process
    void checkContentChanged();

    // called when content is changed by other process
    // doesn't guarantee real-time notification
    static void registerContentChangeHandler(mmkv::ContentChangeHandler handler);
    static void unRegisterContentChangeHandler();

    // by default MMKV will discard all datas on failure
    // return `OnErrorRecover` to recover any data from file
    static void registerErrorHandler(mmkv::ErrorHandler handler);
    static void unRegisterErrorHandler();

    static void setLogLevel(MMKVLogLevel level);

    static void registerLogHandler(mmkv::LogHandler handler);
    static void unRegisterLogHandler();

    static bool isFileValid(const std::string &mmapID, MMKV_PATH_TYPE *relatePath = nullptr);

    // just forbid it for possibly misuse
    explicit MMKV(const MMKV &other) = delete;
    MMKV &operator=(const MMKV &other) = delete;
};

MMKV_NAMESPACE_END

#endif // MMKV_MMKV_H
