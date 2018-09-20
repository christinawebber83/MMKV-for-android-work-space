[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/Tencent/MMKV/pulls) [![Release Version](https://img.shields.io/badge/release-1.0.10-brightgreen.svg)](https://github.com/Tencent/MMKV/releases) [![WeChat Approved iOS](https://img.shields.io/badge/Wechat_Approved_iOS-1.0.10-brightgreen.svg)](https://github.com/Tencent/MMKV/blob/master/iOS/readme.md) [![WeChat Approved Android](https://img.shields.io/badge/Wechat_Approved_Android-1.0.10-brightgreen.svg)](https://github.com/Tencent/MMKV/blob/master/Android/readme.md) [![Platform](https://img.shields.io/badge/Platform-%20iOS%20%7C%20Android-brightgreen.svg)](https://github.com/Tencent/MMKV/wiki/home)

中文版本请参看[这里](./readme_cn.md)

MMKV is an **efficient**, **samll**, **easy-to-use** mobile key-value storage framework used in the WeChat application. It's currently available on both **iOS** and **Android**.


# MMKV for iOS

## Features

* **Efficient**. MMKV uses mmap to keep memory synced with file, and protobuf to encode/decode values, making the most of iOS to achieve best performance.
 
* **Easy-to-use**. You can use MMKV as you go, no configurations needed. All changes are saved immediately, no `synchronize` calls needed.

* **Small**
  * **A handful files**: MMKV contains encode/decode helpers and mmap logics and nothing more. It's really tidy.
  * **Less than 30K in binary size**: MMKV adds less than 30K per architecture on App size, and much less when zipped (ipa).

## Getting Started

### Installation Via CocoaPods:
  1. Install [CocoaPods](https://guides.CocoaPods.org/using/getting-started.html);
  2. Open terminal, `cd` to your project directory, run `pod repo update` to make CocoaPods aware of the latest available MMKV versions;
  3. Edit your Podfile, add `pod 'MMKV'` to your app target;
  4. Run `pod install`;
  5. Open the `.xcworkspace` file generated by CocoaPods;
  6. Add `#import <MMKV/MMKV.h>` to your source file and we are done.

For other installation options, see [iOS Setup](https://github.com/Tencent/MMKV/wiki/iOS_setup).

### Quick Tutorial
You can use MMKV as you go, no configurations needed. All changes are saved immediately, no `synchronize` calls needed.

```objective-c
MMKV *mmkv = [MMKV defaultMMKV];
    
[mmkv setBool:YES forKey:@"bool"];
BOOL bValue = [mmkv getBoolForKey:@"bool"];
    
[mmkv setInt32:-1024 forKey:@"int32"];
int32_t iValue = [mmkv getInt32ForKey:@"int32"];
    
[mmkv setObject:@"hello, mmkv" forKey:@"string"];
NSString *str = [mmkv getObjectOfClass:NSString.class forKey:@"string"];
```

Full tutorials can be found [here](https://github.com/Tencent/MMKV/wiki/iOS_tutorial).

## Performence
Writing random `int` for 10000 times, we get this chart:  
![](https://github.com/Tencent/MMKV/wiki/assets/profile_mini.jpg)  
For more benchmark data, please refer to [our benchmark](https://github.com/Tencent/MMKV/wiki/iOS_benchmark).

# MMKV for Android

## Features

* **Efficient**. MMKV uses mmap to keep memory synced with file, and protobuf to encode/decode values, making the most of Android to achieve best performance.
  * **Multi-Process concurrency**: MMKV supports concurrent read-read and read-write access between processes.

* **Easy-to-use**. You can use MMKV as you go. All changes are saved immediately, no `sync`, no `apply` calls needed.

* **Small**.
  * **A handful files**: MMKV contains process locks, encode/decode helpers and mmap logics and nothing more. It's really tidy.
  * **About 60K in binary size**: MMKV adds about 60K per architecture on App size, and much less when zipped (apk).


## Getting Started

### Installation Via Maven
Add the following lines to `build.gradle` on your app module:

```gradle
dependencies {
    implementation 'com.tencent:mmkv:1.0.10'
    // replace "1.0.10" with any available version
}
```

For other installation options, see [Android Setup](https://github.com/Tencent/MMKV/wiki/android_setup).

### Quick Tutorial
You can use MMKV as you go. All changes are saved immediately, no `sync`, no `apply` calls needed.  
Setup MMKV on App startup, say your `MainActivity`, add these code:

```Java
protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    String rootDir = MMKV.initialize(this);
    System.out.println("mmkv root: " + rootDir);
    //……
}
```

MMKV has a global instance, you can use it directly:

```Java
import com.tencent.mmkv.MMKV;
    
MMKV kv = MMKV.defaultMMKV();

kv.encode("bool", true);
boolean bValue = kv.decodeBool("bool");

kv.encode("int", Integer.MIN_VALUE);
int iValue = kv.decodeInt("int");

kv.encode("string", "Hello from mmkv");
String str = kv.decodeString("string");
```

MMKV also supports **Multi-Process Access**. Full tutorials is here [Android Tutorial](https://github.com/Tencent/MMKV/wiki/android_tutorial).

## Performence
Writing random `int` for 1000 times, we get this chart:  
![](https://github.com/Tencent/MMKV/wiki/assets/profile_android_mini.jpg)  
For more benchmark data, please refer to [our benchmark](https://github.com/Tencent/MMKV/wiki/android_benchmark).

# FAQ
Full [FAQ](https://github.com/Tencent/MMKV/wiki/FAQ).
