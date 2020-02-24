use diesel::prelude::*;
use serde::{Serialize, Deserialize};
use crate::tracker::*;
use crate::schema::*;
use crate::error::*;
use crate::common::*;

#[derive(Queryable, Serialize, Deserialize, Clone)]
pub struct Seeder {
    hash     : String,
    ip       : String,
    port     : i32,
    access   : i32,
}

#[derive(Queryable, Serialize, Deserialize)]
pub struct SeederBaseInfo {
    hash     : String,
    ip       : String,
    port     : i32,
}

#[derive(Insertable)]
#[table_name="seeders"]
pub struct SeederInsert {
    hash        : String,
    ip          : String,
    port        : i32,
}

impl Seeder {
    pub fn get(hash : &str) -> Result<Vec<SeederBaseInfo>> {
        let conn = establish_connection()?;
        let seeder = seeders::table.filter(seeders::hash.eq(hash.clone()))
            .select((seeders::hash, seeders::ip, seeders::port))
            .load::<SeederBaseInfo>(&conn)?;
        Ok(seeder)
    }

    // process the heart_beat pack
    pub fn heart_beat(hash : &str, address : &str, port : i32) -> Result<()> {
        let conn = establish_connection()?;
        let data = (
            seeders::hash.eq(hash.to_string()),
            seeders::ip.eq(address.to_string()), 
            seeders::port.eq(port)
        );
        conn.transaction::<_, diesel::result::Error, _> (|| {
            diesel::insert_or_ignore_into(seeders::table)
                .values(&data)
                .execute(&conn)?;
            diesel::update(
                    seeders::table.filter(seeders::hash.eq(hash.clone()))
                        .filter(seeders::ip.eq(address.clone()))
                        .filter(seeders::port.eq(port))
                )
                .set(seeders::access.eq(1))
                .execute(&conn)?;
            Ok(())
        })?;
        Ok(())
    }

    pub fn delete_dead_connections() -> Result<()> {
        // delete all of the inactive
        let connection = establish_connection()?;
        connection.transaction::<_, diesel::result::Error, _> (|| {
            diesel::delete(seeders::table.filter(seeders::access.eq(0)))
                .execute(&connection)?;
            diesel::update(seeders::table)
                .set(seeders::access.eq(0))
                .execute(&connection)?;
            Ok(())
        })?;
        Ok(())
    }

    pub fn insert(hash : &str, ip : &str, port : i32) -> Result<()> {
        let connection = establish_connection()?;
        let data = (
            seeders::hash.eq(hash.to_string()),
            seeders::ip.eq(ip.to_string()), 
            seeders::port.eq(port)
        );
        diesel::insert_or_ignore_into(seeders::table)
            .values(&data)
            .execute(&connection)?;
        Ok(())
    }

}
