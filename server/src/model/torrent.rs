use diesel::prelude::*;
use serde::{Serialize, Deserialize};
use crate::tracker::*;
use crate::schema::*;
use crate::error::*;
use crate::common::*;

#[derive(Queryable, Insertable, Serialize, Deserialize, Clone)]
pub struct Torrent {
    pub hash     : String,
    pub filename : String,
    pub description : String,
    pub filesize : i64,
    pub index    : String,
}

#[derive(Queryable, Serialize, Deserialize)]
pub struct TorrentBaseInfo {
    hash     : String,
    filename : String,
    description : String,
    filesize : i64,
}

#[derive(Serialize, Deserialize)]
pub struct TorrentData {
    hash     : String,
    filename : String,
    description : String,
    filesize : String,
    count    : String,
}


impl Torrent {
    pub fn get(hash : &str) -> Result<Option<TorrentInfo>> {
        let conn = establish_connection()?;
        let torrent = torrents::table.filter(torrents::hash.eq(hash.clone()))
            .load::<Torrent>(&conn)?;
        if torrent.is_empty() {
            Ok(None)
        } else {
            Ok(Some(TorrentInfo::from_torrent(torrent[0].clone())))
        }
    }

    pub fn insert_torrent(torrent : Torrent) -> Result<()> {
        let conn = establish_connection()?;
        diesel::insert_or_ignore_into(torrents::table)
            .values(torrent)
            .execute(&conn)?;
        Ok(())
    }

    pub fn from_torrent_info(torrent_info : TorrentInfo) -> Result<Self> {
        Ok(Torrent {
            hash        : torrent_info.hash,
            filename    : torrent_info.filename,
            description : torrent_info.description,
            filesize    : torrent_info.filesize.parse::<i64>().unwrap(),
            index       : torrent_info.index,
        })
    }

    pub fn search(search_string : &str) -> Result<Vec<TorrentData>> {
        let conn = establish_connection()?;
        let mut ans = Vec::new();
        let torrents = torrents::table.filter(torrents::filename.like(format!("%{}%", search_string)))
            .select((torrents::hash, torrents::filename, torrents::description, torrents::filesize))
            .load::<TorrentBaseInfo>(&conn)?;
        for each in torrents {
            let count : i64 = seeders::table.filter(seeders::hash.eq(each.hash.clone()))
                .count()
                .first(&conn)?;
            ans.push(TorrentData {
                hash        : each.hash,
                filename    : each.filename,
                description : each.description,
                filesize    : each.filesize.to_string(),
                count       : count.to_string(),
            });
        }
        Ok(ans)
    }
}


