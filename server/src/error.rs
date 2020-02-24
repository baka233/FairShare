use diesel;
use crate::common::{ReplyMsg};
use rocket_contrib::json::Json;
use base64::DecodeError;


#[derive(Responder, Debug)]
#[response(status=200, content_type="json")]
pub enum FireShareError {
    WrongPasswordError(Json<ReplyMsg<String>>),
    TorrentHasExistError(Json<ReplyMsg<String>>),
    TorrentNotExistError(Json<ReplyMsg<String>>),
    QueryError(Json<ReplyMsg<String>>),
    ConnectionError(Json<ReplyMsg<String>>),
    DecodeError(Json<ReplyMsg<String>>),
}

impl From<diesel::result::Error> for FireShareError {
    fn from(err : diesel::result::Error) -> Self {
        FireShareError::QueryError(Json(ReplyMsg::error(502, format!("{}", err))))
    }
}

impl From<diesel::result::ConnectionError> for FireShareError {
    fn from(err : diesel::result::ConnectionError) -> Self {
        FireShareError::ConnectionError(Json(ReplyMsg::error(502, format!("{}", err))))
    }
}

impl From<base64::DecodeError> for FireShareError {
    fn from(err : base64::DecodeError) -> Self {
        FireShareError::ConnectionError(Json(ReplyMsg::error(502, format!("{}", err))))
    }
}

impl FireShareError {
    pub fn wrong_password_error() -> Self {
        FireShareError::WrongPasswordError(Json(ReplyMsg::error(502, "wrong password".to_string())))
    }
}

impl FireShareError {
    pub fn torrent_has_exist() -> Self {
        FireShareError::WrongPasswordError(Json(ReplyMsg::error(502, "torrent has exist".to_string())))
    }
}

impl FireShareError {
    pub fn torrent_not_exist() -> Self {
        FireShareError::WrongPasswordError(Json(ReplyMsg::error(404, "torrent not exist".to_string())))
    }
}


pub type Result<T> = std::result::Result<T, FireShareError>;
