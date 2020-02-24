-- Your SQL goes here
CREATE TABLE torrents (
    hash        CHAR(128) NOT NULL,
    filename    VARCHAR(255) NOT NULL,
    description VARCHAR(255) NOT NULL DEFAULT '',
    filesize    BIGINT NOT NULL,
    `index`     TEXT NOT NULL,
    PRIMARY KEY(hash)
) ENGINE=InnoDB;

