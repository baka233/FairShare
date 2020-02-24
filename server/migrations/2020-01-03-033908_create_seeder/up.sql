-- Your SQL goes here
CREATE TABLE seeders (
    hash        CHAR(128) NOT NULL,
    ip          CHAR(128) NOT NULL,
    port        INTEGER NOT NULL,
    `access`     INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(hash, ip, port),
    CONSTRAINT `seeders_constraint` FOREIGN KEY torrents(`hash`)
    REFERENCES torrents(`hash`)
    ON UPDATE CASCADE
    ON DELETE CASCADE
) ENGINE=InnoDB;
