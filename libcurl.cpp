#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Read the file content into a string
std::string read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

// Callback function for reading data
size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    std::string *upload = (std::string*)userp;
    size_t buffer_size = size * nmemb;
    
    if (upload->size() == 0)
        return 0;
    
    size_t copy_size = (upload->size() > buffer_size) ? buffer_size : upload->size();
    memcpy(ptr, upload->c_str(), copy_size);
    upload->erase(0, copy_size);
    
    return copy_size;
}

int main() {
    CURL *curl;
    CURLcode res = CURLE_OK;

    struct curl_slist *recipients = NULL;
    std::string email_content = "From: Your Name <your_email@example.com>\r\n"
                                "To: Recipient Name <recipient@example.com>\r\n"
                                "Subject: Daily Report\r\n"
                                "\r\n"
                                "This is the body of the email.\r\n";

    // Read the attachment file content
    std::string file_content = read_file("path/to/your/file.txt");

    // Construct the email payload with the attachment
    std::ostringstream email_payload;
    email_payload << email_content
                  << "\r\n--boundary\r\n"
                  << "Content-Type: text/plain\r\n"
                  << "Content-Disposition: attachment; filename=\"file.txt\"\r\n"
                  << "\r\n"
                  << file_content
                  << "\r\n--boundary--\r\n";

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, "your_email@example.com");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "your_password");
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.example.com:587");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "your_email@example.com");

        recipients = curl_slist_append(recipients, "recipient@example.com");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_payload.str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    return (int)res;
}
