【Demo功能】
1、Demo介绍SDK初始化、登陆设备、登出设备、录像下载、录像回放和录像倒放。
2、录像下载包括：按时间下载。Demo演示了下载，停止下载等功能
3、录像回放包括：按时间回放，回放时可以进行快放，慢放，暂停，恢复，前进，后退等功能。
4、录像倒放包括：按时间倒放，倒放时可以进行快放，慢放，暂停，恢复，前进，后退等功能。

【注意事项】
1、编译环境为VS2005。
2、若回放时无画面，请确认是否添加PlaySDK的DLL。
3、本demo录像回放只展示了单个设备单个通道的录像回放，若要回放多通道视频录像，请自行添加代码。
4、本demo只支持二期录像倒放，但是目前支持的设备较少，不支持二期倒放的设备无法使用该demo进行倒放操作；
5、本demo在以下条件下，回放、倒放等功能可正常运行，其他环境下可能出现异常：
	（1）最大支持分辨率1920*1080
	（2）码流格式为H264
	（3）最大支持帧率25
	（4）最大支持4M固定码流
    若想要进行适当的放开这个限制，可以调用CLIENT_SetNetworkParam，将byPlaybackBufSize扩大，默认为4M
6、如需运行demo的exe文件，请将开发包中【库文件】里的全部dll文件拷贝至该工程目录下的bin/x86release或者bin/x64release下。
7、如把库文件放入程序生成的目录中，运行有问题，请到大华官网下载最新的网络SDK版本：http://www.dahuatech.com/index.php/service/downloadlists/836.html 替换程序中的库文件

【Demo Features】
1.Demo SDK initialization,login device,logout device,auto reconnect device, listen alarm information, receive alarm information, parse alarm information, stop listen alarm information function.
2.Demo external alarm, motion detection alarm,video loss alarm,camera masking alarm,HDD full alarm,HDD serror alarm.

【NOTE】
1.Complier for Demo is VS2005.
2.Just only support for listening to single device,not support for listening to multiple devices.Modify the code if the user has a requirement.
3.Just only demo gengral alarm function, add others alarm code if the user has a requirement.
4.This demo only supports second-stage video playback,but currwntly it supports fewer devices.Devices that do not support second-stage video playback can not use this demo for playback operation.
5.Under the following conditions,the functions of playback forward/playback backward and so on can be operated normally.Exceptions may occur in other environments.
	(1)Maximum support resolution 1920*1080
	(2)Support H264
	(3)Maximum frame is 25
	(4)Maximum support 4M fixed bitstream.
    If you want to properly let go of this restriction, you can call CLIENT_SetNetworkParam，to expand the byPlaybackBufSize.
6.If you want to run the demo exe file, please copy all dll files in the development package [Bin] to bin/x86release or bin/x64release under the project directory
7.If run the program with some problems,please go to http://www.dahuasecurity.com/download_3.html download the newest version,and replace the DLL files

