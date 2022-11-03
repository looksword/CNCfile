# 编译curl到windows(支持sftp)

下载zlib（ref:https://blog.csdn.net/qq292386890/article/details/49620633?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~Rate-2-49620633-blog-104270047.pc_relevant_multi_platform_whitelistv3&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2~default~CTRLIST~Rate-2-49620633-blog-104270047.pc_relevant_multi_platform_whitelistv3&utm_relevant_index=5）  
管理员模式运行VS2013 x86 本机工具命令提示  
切换到../zlib-1.2.11/contrib/masmx86, 运行bld_ml32.bat  
切换到../zlib-1.2.11/  
nmake -f win32/Makefile.msc LOC="-DASMV -DASMINF" OBJA="inffas32.obj match686.obj"  




下载openssl（ref:https://blog.csdn.net/liang19890820/article/details/51658574)  
打开cmd  
切换到../openssl-OpenSSL_1_0_2  
perl Configure VC-WIN32 --prefix=../openssl(将其安装到../openssl)  
ms\do_nasm  
切换到../Microsoft Visual Studio 12.0\VC\bin  
vcvars32.bat  
切换到../openssl-OpenSSL_1_0_2  
nmake -f ms\ntdll.mak  
nmake -f ms\ntdll.mak test(显示 passed all tests 则说明生成的库正确)  
nmake -f ms\ntdll.mak install  
(在../openssl下生成 bin、include、lib、ssl 四个文件夹)  
(编译静态库，则用 ms\nt.mak 而不使用 ms\ntdll.mak )  




下载libssh2
打开cmd  
切换到../libssh2-libssh2-1.9.0
cmake -DCRYPTO_BACKEND=WinCNG -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=../dll -G="Visual Studio 12 2013"
打开 ../libssh2-libssh2-1.9.0/libssh2.sln
生成libssh2项目
../libssh2-libssh2-1.9.0\src\Release 里生成库
头文件使用 ../libssh2-libssh2-1.9.0\include 里的




下载curl
新建几个目录，里面放openssl和libssh2的库和头文件(include文件夹和lib文件夹)
nmake /f Makefile.vc mode=dll MACHINE=x86 WITH_SSL=static SSL_PATH=..\openssl WITH_SSH2=static SSH2_PATH=..\libssh2
将在 ..\curl-7.83.1\builds\libcurl-vc-x86-release-dll-ssl-static-ssh2-static-ipv6-sspi\bin 里生成curl.exe
缺失的dll可以复制过来(libeay32.dll、libssh2.dll、ssleay32.dll)


