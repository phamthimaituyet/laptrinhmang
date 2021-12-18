<?php

session_start();
$host="127.0.0.1";
$port = 5500;
$sock = socket_create(AF_INET, SOCK_STREAM, 0);
socket_connect($sock, $host, $port);
$_SESSION["socker"] = $sock;
$_SESSION["123"] = 12345;