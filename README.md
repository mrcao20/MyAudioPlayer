# MyAudioPlayer
删除原有仓库，重构仓库，并新增下载功能，暂时只有QQ音乐的下载
# 2018.8.31
完善了下载功能，网易云音乐亦可下载，但和播放连接为两个不同连接
添加简单的AES加密算法和利用python简单实现的RSA加密算法，以向网易云服务器请求并获取音乐源地址

优化部分代码和功能

# 2018-9-23
优化代码，使其在没有数据库的情况下仍能运行
增加接收windows消息，使其在电脑关机时能做出相应操作
增加socket编程方式，使所有输出消息发送到内置服务端，进一步显示到管理进程输出端
