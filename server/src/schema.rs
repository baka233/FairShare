table! {
    seeders (hash, ip, port) {
        hash -> Char,
        ip -> Char,
        port -> Integer,
        access -> Integer,
    }
}

table! {
    torrents (hash) {
        hash -> Char,
        filename -> Varchar,
        description -> Varchar,
        filesize -> Bigint,
        index -> Text,
    }
}

joinable!(seeders -> torrents (hash));

allow_tables_to_appear_in_same_query!(
    seeders,
    torrents,
);
