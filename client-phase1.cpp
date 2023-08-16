#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void print_ip_host(struct addrinfo *res)
{
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *ser = (struct sockaddr_in *)res->ai_addr;
    void *addr = &(ser->sin_addr);
    inet_ntop(AF_INET, addr, ipstr, sizeof ipstr);

    cout << ipstr << endl;
}

int send_msg(const char *msg, int sfd)
{
    int len = strlen(msg), b_sent = 0;
    while (b_sent != len)
    {

        int p_sent = send(sfd, &msg[b_sent], len - b_sent, 0);
        if (p_sent == -1)
            return -1;
        b_sent += p_sent;
    }
    return b_sent;
}

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *diread;
    vector<string> files;

    if ((dir = opendir(argv[2])) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            string kkk = diread->d_name;
            if (kkk == "." || kkk == ".." || kkk == "Downloaded")
                continue;
            files.push_back(kkk);
        }
        closedir(dir);
    }
    else
    {
        perror("opendir");
        return EXIT_FAILURE;
    }
    sort(files.begin(), files.end());
    for (auto file : files)
        cout << file << "\n"; // printing the files in directory

    ifstream File;
    File.open(argv[1]);

    string line;
    vector<string> testcase;

    while (File.good())
    {
        File >> line;
        testcase.push_back(line);
    }
    int id = stoi(testcase[0]);
    string port = testcase[1];
    string uid = testcase[2];
    int neigh = stoi(testcase[3]);
    int neigh_id[neigh];
    string neigh_port[neigh];
    for (int i = 0; i < neigh; i++)
    {
        neigh_id[i] = stoi(testcase[4 + 2 * i]);
        neigh_port[i] = testcase[5 + 2 * i];
    }
    vector<string> search;
    int search_ka_size = stoi(testcase[4 + 2 * neigh]);
    for (int i = 5 + 2 * neigh; i < testcase.size(); i++)
    {
        if (!search_ka_size)
            break;
        search.push_back(testcase[i]);
        search_ka_size--;
    }
    sort(search.begin(), search.end());

    /// testcases and files reading done///

    int sockserver, server, sockclient[neigh], socknew[neigh];
    struct addrinfo hints, *res;
    struct addrinfo *cres[neigh];

    memset(&hints, 0, sizeof hints);
    memset(sockclient, 0, sizeof(int) * neigh);
    hints.ai_family = AF_INET;
    // hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("127.0.0.1", port.c_str(), &hints, &res) != 0)
    {
        cout << "Error(97): getaddrinfo()\n";
        return 0;
    }

    if (res == nullptr)
    {
        cout << "Error(103): res NULL\n";
        return 0;
    }

    if ((sockserver = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        cout << "Error(109): socket error\n";
        return 0;
    }

    int enable_multi = 1;
    if (setsockopt(sockserver, SOL_SOCKET, SO_REUSEADDR, &enable_multi, sizeof enable_multi) == -1)
    {
        close(sockserver);
        cout << "Error(117): Reuse Failed\n";
    }

    if (bind(sockserver, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockserver);
        cout << "Error(122): bind error\n";
        return 0;
    }

    for (int i = 0; i < neigh; i++)
    {
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        // cout << neigh_port[i].c_str() << " port---------------------------\n";
        if (getaddrinfo("127.0.0.1", neigh_port[i].c_str(), &hints, &cres[i]) != 0)
        {
            cout << "Error(134): getaddrinfo() failed on client " << neigh_id[i] << endl;
            return 0;
        }

        if (cres[i] == nullptr)
        {
            cout << "Error(140): res NULL on client " << neigh_id[i] << endl;
            return 0;
        }
        // cout << "Connect: Port " << ((struct sockaddr_in *)&cres[i]->ai_addr)->sin_port << " Neigh " << i << endl;

        if ((sockclient[i] = socket(cres[i]->ai_family, cres[i]->ai_socktype, cres[i]->ai_protocol)) == -1)
        {
            cout << "Error(146): socket failed on client " << neigh_id[i] << endl;
            return 0;
        }
    }

    if (listen(sockserver, neigh + 1) == -1)
    {
        cout << "Error(153): Listen failed\n";
    }

    int tmpc = neigh, tmps = neigh;
    bool con_cl[neigh] = {0}, con_sv[neigh] = {0};
    struct sockaddr their_addr;
    char c_uid[neigh][10];
    int rev_b[neigh] = {0};
    while (true)
    {
        for (int i = 0; i < neigh; i++)
        {
            if (!(con_cl[i]) && !(connect(sockclient[i], cres[i]->ai_addr, cres[i]->ai_addrlen) == -1))
            {

                tmpc--;
                con_cl[i] = 1;
                // cout << "Connected " << i << endl;
            }
        }
        if (tmpc == 0 && tmps == 0)
            break;
        fd_set readfds;
        int maxfd = sockserver;
        FD_ZERO(&readfds);
        FD_SET(sockserver, &readfds);
        char buffer[INET_ADDRSTRLEN];

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1)
        {
            cout << "Error(179): select failed\n";
            return 0;
        }
        // cout << "Select passed\n";
        if (FD_ISSET(sockserver, &readfds))
        {
            memset(&their_addr, 0, sizeof their_addr);
            socklen_t addrlen = sizeof their_addr;
            int newfd = accept(sockserver, &their_addr, &addrlen);
            char clt[INET_ADDRSTRLEN], serv[10];
            if (newfd == -1)
            {
                cout << "Error(192): Accept failed\n";
                return 0;
            }
            string x = inet_ntop(AF_INET, &(((struct sockaddr_in *)&their_addr)->sin_addr), buffer, INET_ADDRSTRLEN);
            uint16_t remoteport = ntohs(((struct sockaddr_in *)&their_addr)->sin_port);

            socknew[--tmps] = newfd;
        }
    }

    // uid sending part
    string senduid = uid + " ";
    for (int i = 0; i < neigh; i++)
    {
        if (send_msg(senduid.c_str(), socknew[i]) == -1)
        {
            cout << "Error(240): Sending failed for index " << i << endl;
            return 0;
        }
    }

    int net_rec = neigh;
    // uid receiving part
    while (true)
    {
        if (!neigh)
            break;
        fd_set readfds;
        int max_fds = 0;
        FD_ZERO(&readfds);
        for (int i = 0; i < neigh; i++)
        {
            FD_SET(sockclient[i], &readfds);
            max_fds = max(max_fds, sockclient[i]);
        }

        if (select(max_fds + 1, &readfds, NULL, NULL, NULL) == -1)
        {
            cout << "Error(261): Read Select failed\n";
            return 0;
        }

        for (int i = 0; i < neigh; i++)
        {
            if (FD_ISSET(sockclient[i], &readfds))
            {
                int recd = recv(sockclient[i], &c_uid[i][rev_b[i]], sizeof(char) * (10 - rev_b[i]), 0);
                if (recd < 0)
                {
                    cout << "Error(273): Receive failed on index " << i << endl;
                    return 0;
                }
                else if (recd == 0)
                {
                    // cout << "Msg: Connection closed on index " << i << endl;
                }
                else
                {
                    rev_b[i] += recd;
                    if (c_uid[i][rev_b[i] - 1] == ' ')
                    {
                        c_uid[i][rev_b[i] - 1] = '\0';
                        net_rec--;
                    }
                }
            }
        }

        if (net_rec <= 0)
            break;
    }


    for (int i = 0; i < neigh; i++)
    {
        cout << "Connected to " << neigh_id[i] << " with unique-ID " << c_uid[i] << " on port " << neigh_port[i] << endl;
    }
    return 0;
}
