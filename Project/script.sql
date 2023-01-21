CREATE DATABASE IF NOT EXISTS Caro;
USE Caro;

DROP TABLE IF EXISTS players;

CREATE TABLE players (
  id int primary key auto_increment,
  username varchar(50) not null,
  password varchar(63) not null,
  win int not null default 0,
  draw int not null default 0,
  loss int not null default 0,
  streak int not null default 0,
  points int not null default 0
);