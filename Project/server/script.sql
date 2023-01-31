CREATE DATABASE IF NOT EXISTS Caro;
USE Caro;

DROP TABLE IF EXISTS players;

CREATE TABLE players (
  id int primary key auto_increment,
  username varchar(50) not null,
  password varchar(64) not null,
  avatar varchar(255) not null,
  game int not null default 0,
  win int not null default 0,
  draw int not null default 0,
  loss int not null default 0,
  streak int not null default 0,
  points int not null default 0
);
