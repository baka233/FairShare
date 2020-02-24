#![feature(proc_macro_hygiene, decl_macro)]
#[macro_use] 
extern crate rocket;
#[macro_use] 
extern crate diesel;
mod error;
mod common;
mod schema;
mod model;
mod tracker;
mod seeder;
//use crate::table::*;
use crate::tracker::*;
use crate::seeder::*;
use crate::model::seeder::*;
use tokio::time::{Duration};
use tokio::task;
use tokio::prelude::*;
use tokio::runtime::Runtime;

fn main() {
    let rt = Runtime::new().unwrap();
    rt.spawn(async {
        let mut _task = tokio::time::interval(Duration::from_secs(10));
        loop {
            _task.tick().await;  
            Seeder::delete_dead_connections();
        }
    });

    rocket::ignite().mount("/", routes![
        get_torrent,
        upload_torrent,
        search_torrent,
        get_seeder_list,
        fresh_seeder,
    ]).launch();
}
