#ifndef TASK_H
#define TASK_H
#include <string>
#include "fileindex.h"
#include "worker.h"
#include <fstream>
#include <memory>
#include <QTcpServer>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QDateTime>
#include <QtNetwork/QNetworkAccessManager>
#include "downloadworker.h"
#include "fileverifyworker.h"


class Task : public QObject, std::enable_shared_from_this<Task>
{
    Q_OBJECT
public:
    enum TaskStartStatus {
    	DOWNLOAD,
        UPLOAD
    };
    enum TaskStatus {
    	DOWNLOADING,
        READY,
        SEEDING,
        VERIFYING,
        PAUSED,
        FAILED,
        DELETED,
    };

    enum UploadStatus {
        CAN_SEEDING,
        NOT_READY,
    };

    enum BlockStatus {
        BLOCK_EMPTY,
        BLOCK_FULL,
        BLOCK_LOCK,
    };

    explicit Task(QString _file_path, QString _info, TaskStartStatus _type, QObject* parent = nullptr);

    inline uint64_t get_downloaded_size() const {
        return this->downloaded_size;
    }

    inline QString get_downloaded_file_size() const {
        return size_beautify(this->real_readed);
    }

    inline QString get_uploaded_file_size() const {
        return size_beautify(this->uploaded);
    }

    void remove_block_lock(uint64_t block_num);

    Task(const Task&) = delete;

    ~Task();
    
    void start();
    void pause();
    void upload();
    void download();
    inline TaskStatus get_status() const {
    	return this->status;
    }
    bool set_status(Task::TaskStatus new_status) {
        if (status != TaskStatus::FAILED) {
            status = new_status;
            emit status_change();
        }
        return true;
    }
    
    inline QString get_filename() const {
    	return this->filename;  
    }
    inline QString get_md5() const {
        return this->md5sum;
    }
    inline unsigned long long get_size() const {
        return this->size;
    }

    const QVector<uint64_t>& get_full_blocks();
    QString get_status_string() const;
    double get_speed(uint64_t num, uint64_t time) const {
        return num * 1.0 / (time * 1.0 / 1000);
    }
    QString get_speed_string(double speed) const;
    QString get_size_beautify() const;
    bool pull_ip_list();
    bool try_to_check_block(uint64_t block_num);
    QByteArray calculate_sum(std::ifstream& stream);
    bool get_block(uint64_t block_num, uint32_t* read_num, char* array);

    inline QSet<std::pair<QString, int>>& get_seeders() {
        return seeders;
    }
    bool check_block(uint64_t block_num, QByteArray& bytes);
    bool set_block_full(uint64_t block);
    inline void add_readed(uint64_t size) {
        readed += size;
        real_readed += size;
    }

    inline void add_uploaded(uint64_t size) {
        uploaded += size;
    }

    inline void sub_real_readed(uint64_t size) {
        real_readed -= size;
    }

    QString calculate_download_speed(uint64_t time) {
        uint64_t num = readed - pre_readed;
        current_download_speed = get_speed(num, time);
        auto ans = get_speed_string(current_download_speed);
        pre_readed = readed;
        return ans;
    }

    QString get_download_speed() {
        return get_speed_string(current_download_speed);
    }

    QString get_upload_speed() {
        return get_speed_string(current_upload_speed);
    }

    double get_progress() {
        uint64_t downloaded = real_readed > (downloaded_size * per_block_size) ? real_readed : (downloaded_size * per_block_size);
        double progress = downloaded * 1.0 / size;
        if (progress >= 1) {
            progress = 100;
        } else {
            progress = 100 * progress;
        }
        return progress;
    }

    QString calculate_uploaded_speed(uint64_t time) {
        uint64_t num = uploaded - pre_uploaded;
        current_upload_speed = get_speed(num, time);
        auto ans = get_speed_string(current_upload_speed);
        pre_uploaded = uploaded;
        return ans;
    }

    QString get_remain_time();

    void set_uuid(QString uuid) {
        this->uuid = uuid;
    }

    QString get_uuid() {
        return this->uuid;
    }

    QString file_path;

    uint64_t get_seeders_number() {
        return this->seeders.size();
    }

    uint64_t get_block_number() {
        return block_size;
    }

    QString get_add_time() {
        if (add_time.isValid()) {
            return add_time.toString();
        }
        return "";
    }

    QString get_finish_time() {
        if (finish_time.isValid()) {
            return finish_time.toString();
        }
        return "";
    }

    QString get_file_path() {
        return file_path;
    }

    QString get_comment() {
        return description;
    }

private:
    QString size_beautify(uint64_t _size) const;
    int pull_info();
    int put_info();
    void change_status();

    QDateTime add_time;
    QDateTime finish_time;
    uint64_t readed = 0;
    uint64_t real_readed = 0;
    uint64_t pre_readed = 0;
    uint64_t uploaded = 0;
    uint64_t pre_uploaded = 0;
    double current_upload_speed = 0;
    double current_download_speed = 0;
    QMutex status_lock;
    DownloadWorker* worker = nullptr;
    FileVerifyWorker* file_verify_worker = nullptr;
    QSet<std::pair<QString, int>> seeders;
    QVector<QTcpSocket> download_scokets;
    QMutex bitmap_lock;
    QVector<uint64_t> downloaded_blocks;
    QVector<uint8_t> bitmap;
    QTcpServer* server = nullptr;
    QString description = "";
    QString base_url = "http://127.0.0.1:8000";
    enum TaskStatus status;
    QString filename;
    QString uuid;
    uint64_t size = 0;
    uint64_t block_size;
    uint64_t downloaded_size = 0;
    TaskStartStatus type = TaskStartStatus::UPLOAD;
    uint64_t speed;
    QString md5sum = "";
    QVector<std::shared_ptr<Worker>> worker_list;
    std::shared_ptr<FileIndex> index;

    std::shared_ptr<QNetworkAccessManager> net_manager;

signals:
    void upload_prepared();
    void status_change();
};

#endif // TASK_H
