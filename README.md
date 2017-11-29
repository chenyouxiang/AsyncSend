### get data from php with msg queue and send to httpserver with epoll

### need: 

* gcc 4.8.5

* Cmake

* boost

### compile step:

* cd to the project path

* cmake .

* make

* complie success 

### start the app:
```shell
./async-send --msg-key=1000  --msg-size=10240
```

#### msg-key and msg-size decide by you self

#### the php code a.php:
```php
$msg_id = msg_get_queue(1000); 

msg_send($msg_id,1,'{"host":"localhost:80", "url":"/index.php","body":"testasdfasdg"}',false);
```


#### when execute: 
```shell
php a.php  
```
#### then the c++ app will get the json and send to http://localhost/index.php with content "testasdfasdg"