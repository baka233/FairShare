extern crate diesel;
extern crate dotenv;

use serde::{Serialize, Deserialize};
use diesel::prelude::*;
use diesel::mysql::MysqlConnection;
use dotenv::dotenv;
use crate::error::*;
use std::env;

#[derive(Serialize, Deserialize, Debug)]
pub struct ReplyMsg<T : Serialize> {
    code    : u32,
    data    : T,
    message : String,
}

impl<T : Serialize> ReplyMsg<T> {
    pub fn success(data : T) -> Self {
        ReplyMsg {
            code : 200,
            data,
            message : "".to_string(),
        }
    }

}

impl ReplyMsg<String> {
    pub fn error(code : u32, message : String) -> Self {
        ReplyMsg {
            code,
            data : "".to_string(),
            message,
        }
    }
}

pub fn establish_connection() -> Result<MysqlConnection> {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASSE_URL must be set");
    Ok(MysqlConnection::establish(&database_url)?)
}

