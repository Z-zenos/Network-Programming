CREATE DATABASE IF NOT EXISTS Caro;
USE Caro;

DROP TABLE IF EXISTS players;

CREATE TABLE players (
  id int primary key auto_increment,
  username varchar(50) not null,
  password varchar(64) not null,
  avatar varchar(50) not null,
  game int not null default 0,
  win int not null default 0,
  draw int not null default 0,
  loss int not null default 0,
  streak int not null default 0,
  points int not null default 0
);

CREATE TABLE friends (
 player_id int not null,
 friend_id int not null,
 status tinyint not null,
 primary key (player_id, friend_id),
 CONSTRAINT `FK_FRIENDS_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE,
 CONSTRAINT `FK_FRIENDS_2` FOREIGN KEY (`friend_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE
);

INSERT INTO friends (player_id, friend_id)
VALUES
    (1, 2),
    (1, 3),
    (1, 4),
    (2, 1),
    (3, 1),
    (4, 1),
    (2, 4),
    (4, 2);
