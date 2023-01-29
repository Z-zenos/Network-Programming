CREATE DATABASE IF NOT EXISTS Caro;
USE Caro;

DROP TABLE IF EXISTS players;

CREATE TABLE players (
  id int primary key auto_increment,
  username varchar(50) not null,
  password varchar(64) not null,
  win int not null default 0,
  draw int not null default 0,
  loss int not null default 0,
  streak int not null default 0,
  points int not null default 0
);
934b725da05d3efe57fd23f456dd28b94a8c8f50eef1a941acc6fc71570f5ce