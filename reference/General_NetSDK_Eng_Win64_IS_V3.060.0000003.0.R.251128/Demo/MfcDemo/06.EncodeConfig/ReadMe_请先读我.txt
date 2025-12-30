【Demo功能】
1、demo演示的功能有：设备登陆、实时预览、视屏属性配置、音频属性配置、采样方式配置、抓图配置。
2、demo可设置的码流类型有：主码流、报警码流、动检码流、辅码流1、辅码流2、辅码流3

【注意事项】
1、编译环境为VS2005。
2、点击登陆后demo自动播放第一通道的主码流，界面并无开始播放和停止按钮。退出登陆后demo停止该通道码流播放
3、登陆设备后，demo会获取主码流的视频属性、音频属性、采样属性和抓图属性的配置展示到界面上，选择不同的码流类型就重新获取各个属性配置。
4、如需运行demo的exe文件，请将开发包中【库文件】里的全部dll文件拷贝至该工程目录下的bin/x86release或者bin/x64release下。
5、如把库文件放入程序生成的目录中，运行有问题，请到大华官网下载最新的网络SDK版本：http://www.dahuatech.com/index.php/service/downloadlists/836.html 替换程序中的库文件。

【Demo Features】
1.Demo demo features: device login, real-time monitoring, video properties, audio properties, sampling configuration, capture configuration.
2.Demo can set the type of stream are: the main stream, alarm code stream, check bit stream, sub-stream 1, sub-stream 2, sub-stream 3

【NOTE】
1.Complier for Demo is VS2005.
2.Click on the demo automatically play the first channel of the main stream, the interface does not start playing and stop button. Log out of the demo to stop the channel stream playback
3.After logging in to the device, the demo obtains the configuration of the video attributes, audio attributes, sampling attributes, and capture attributes of the main stream, and displays the configuration of the attribute on the interface by selecting different stream types.
4.If you want to run the demo exe file, please copy all dll files in the development package [Bin] to bin/x86release or bin/x64release under the project directory
5.If run the program with some problems,please go to http://www.dahuasecurity.com/download_3.html download the newest version,and replace the DLL files.
