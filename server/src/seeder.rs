use crate::model::seeder::*;
use rocket_contrib::json::Json;
use serde::{Serialize, Deserialize};
use crate::common::*;
use crate::error::*;

#[derive(Serialize, Deserialize)]
pub struct HeartBeat {
    pub hash : String,
    pub ip   : String,
    pub port : i32,
}

#[get("/seeder/get/<hash>")]
pub fn get_seeder_list(hash : String) -> Result<Json<ReplyMsg<Vec<SeederBaseInfo>>>> {
    Ok(Json(ReplyMsg::success(Seeder::get(&hash)?)))
}

#[post("/seeder/fresh", format = "application/json", data = "<heart_beat>")]
pub fn fresh_seeder(heart_beat : Json<HeartBeat>) -> Result<Json<ReplyMsg<()>>> {
    let heart_beat = heart_beat.into_inner();
    Seeder::heart_beat(&heart_beat.hash, &heart_beat.ip, heart_beat.port)?;
    Ok(Json(ReplyMsg::success(())))
}

