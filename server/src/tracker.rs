use diesel::prelude::*;
use rocket_contrib::json::Json;
use serde::{Serialize, Deserialize};
use crate::model::torrent::*;
use crate::common::*;
use crate::error::*;

#[derive(Serialize, Deserialize)]
pub struct TorrentInfo {
    pub hash : String,
    pub filename : String,
    pub description : String,
    pub filesize : String,
    pub index : String,
}

impl TorrentInfo {
    pub fn from_torrent(torrent : Torrent) -> Self {
        TorrentInfo {
            hash     : torrent.hash,
            filename : torrent.filename,
            description : torrent.description,
            filesize : torrent.filesize.to_string(),
            index    : torrent.index,
        }
    }
}

#[get("/tracker/get/<hash>")]
pub fn get_torrent(hash : String) -> Result<Json<ReplyMsg<TorrentInfo>>> {
    let torrent = Torrent::get(&hash)?;
    if torrent.is_none() {
        Err(FireShareError::torrent_not_exist())
    } else {
        Ok(Json(ReplyMsg::success(torrent.unwrap())))
    }
}

#[post("/tracker/upload", format = "application/json", data = "<torrent_info>")]
pub fn upload_torrent(torrent_info : Json<TorrentInfo>) -> Result<Json<ReplyMsg<()>>>{
    let torrent_info = torrent_info.into_inner();
    let torrent = Torrent::from_torrent_info(torrent_info)?;
    Torrent::insert_torrent(torrent)?;
    Ok(Json(ReplyMsg::success(()))) 
}

#[get("/tracker/search/<search_string>")]
pub fn search_torrent(search_string : String) -> Result<Json<ReplyMsg<Vec<TorrentData>>>> {
   Ok(Json(ReplyMsg::success(Torrent::search(&search_string)?)))
}







