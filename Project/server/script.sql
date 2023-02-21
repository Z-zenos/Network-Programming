DROP DATABASE Caro;

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
    points int not null default 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

UPDATE players SET game = 5, win = 4, loss = 1, points = 12 WHERE id = 1;
UPDATE players SET game = 6, win = 1, draw = 3, loss = 2, points = 6 WHERE id = 2;
UPDATE players SET game = 4, win = 2, draw = 2, points = 8 WHERE id = 3;
UPDATE players SET game = 7, draw = 1, loss = 6, points = 1 WHERE id = 4;
UPDATE players SET game = 4, win = 3, loss = 1, points = 9 WHERE id = 5;

CREATE TABLE friends (
    player_id int not null,
    friend_id int not null,
    confirmed tinyint not null,
    primary key (player_id, friend_id),
    CONSTRAINT `FK_FRIENDS_1` FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE,
    CONSTRAINT `FK_FRIENDS_2` FOREIGN KEY (`friend_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE
);

INSERT INTO friends (player_id, friend_id, confirmed)
VALUES
    (1, 3, 1),
    (1, 4, 1),
    (2, 1, 0),
    (2, 5, 1),
    (3, 4, 1),
    (4, 5, 0),
    (5, 1, 0),
    (2, 4, 1);

CREATE TABLE histories (
    id int primary key not null auto_increment,
    player1_id int not null,
    player2_id int not null,
    result tinyint not null,
    num_moves int not null,
    time DATETIME not null DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT `FK_PLAYER_1` FOREIGN KEY (`player1_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE,
    CONSTRAINT `FK_PLAYER_2` FOREIGN KEY (`player2_id`) REFERENCES `players` (`id`) ON UPDATE CASCADE ON DELETE CASCADE
)
