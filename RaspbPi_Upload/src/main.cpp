#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>
#include <thread>
#include <chrono>

#define UART_DEVICE "/dev/serial0"
#define BAUD_RATE B115200

int uart_fd = -1;

// UART setup
int setup_uart() {
    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Failed to open UART");
        return -1;
    }

    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetospeed(&options, BAUD_RATE);
    cfsetispeed(&options, BAUD_RATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag = 0; // Non-canonical mode
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);

    return 0;
}

// Send raw string to UART
void uart_send(const std::string &cmd) {
    write(uart_fd, cmd.c_str(), cmd.size());
    usleep(100000); // small delay
}

// Read from UART with timeout
bool uart_read(std::string &response, int timeout_ms) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(uart_fd, &set);
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    int rv = select(uart_fd + 1, &set, NULL, NULL, &timeout);
    if (rv > 0) {
        int len = read(uart_fd, buf, sizeof(buf) - 1);
        if (len > 0) {
            response = std::string(buf, len);
            return true;
        }
    }
    return false;
}

// HTTP callback (optional debug)
size_t http_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    fwrite(buffer, size, nmemb, stdout);
    return size * nmemb;
}

// Upload data to Flask API as JSON
void sendDataToFlask(double temp, double humidity, double spo2, double bpm,
                     double ax, double ay, double az, double gx, double gy, double gz) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to init curl\n";
        return;
    }

    // Build JSON string without trailing comma
    std::string jsonData = "{";
    jsonData += "\"field1\":" + std::to_string(temp) + ",";
    jsonData += "\"field2\":" + std::to_string(humidity) + ",";
    jsonData += "\"field3\":" + std::to_string(spo2) + ",";
    jsonData += "\"field4\":" + std::to_string(bpm) + ",";
    jsonData += "\"field5\":" + std::to_string(ax) + ",";
    jsonData += "\"field6\":" + std::to_string(ay) + ",";
    jsonData += "\"field7\":" + std::to_string(az) + ",";
    jsonData += "\"field8\":" + std::to_string(gx) + ",";
    jsonData += "\"field9\":" + std::to_string(gy) + ",";
    jsonData += "\"field10\":" + std::to_string(gz);
    jsonData += "}";

    std::string url = "http://192.168.168.121:5000/api/upload";

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Debug print of outgoing JSON
    std::cout << "Sending JSON: " << jsonData << "\n";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to POST data: " << curl_easy_strerror(res) << "\n";
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        std::cout << "Flask server responded with HTTP " << http_code << "\n";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

int main() {
    if (setup_uart() != 0) return -1;

    std::cout << "UART ready. Attempting RN4871 connection...\n";

    // Retry loop until module responds
    bool connected = false;
    while (!connected) {
        uart_send("$\$\$"); // enter command mode
        std::string resp;
        if (uart_read(resp, 500)) {
            std::cout << "RN4871 response: " << resp << "\n";
            if (resp.find("CMD") != std::string::npos) {
                uart_send("C\r"); // connect to last paired device
                uart_read(resp, 500);
                std::cout << "Connect response: " << resp << "\n";

                uart_send("---\r"); // exit command mode
                uart_read(resp, 500);
                std::cout << "Exit response: " << resp << "\n";

                connected = true;
                std::cout << "RN4871 connected!\n";
            }
        }
        if (!connected) {
            std::cout << "Retrying in 1s...\n";
            sleep(1);
        }
    }

    unsigned char rx_buffer[256];
    auto lastUpdate = std::chrono::steady_clock::now();

    // Main read loop
    while (true) {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        int rx_length = read(uart_fd, rx_buffer, sizeof(rx_buffer) - 1);

        if (rx_length > 0) {
            rx_buffer[rx_length] = '\0';
            std::cout << "Received: " << rx_buffer << "\n";

            double temp, humidity, spo2, bpm, ax, ay, az, gx, gy, gz;
            if (sscanf((const char*)rx_buffer, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                       &temp, &humidity, &spo2, &bpm, &ax, &ay, &az, &gx, &gy, &gz) == 10) {

                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 5) {
                    sendDataToFlask(temp, humidity, spo2, bpm, ax, ay, az, gx, gy, gz);
                    lastUpdate = now;
                }
            }
        }
        usleep(100000); // 100ms delay
    }

    close(uart_fd);
    return 0;
}
