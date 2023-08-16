#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void shiftarr(char *msg, int len, int shift)
{
    for (int i = 0; i < len - shift; i++)
    {
        msg[i] = msg[i + shift];
    }
}

string msg_parse(char *msg, int len)
{
    string cmd = "";
    for (int i = 0; i < len; i++)
    {
        cmd += msg[i];
        if (msg[i] == 3 || msg[i] == 4)
            return cmd;
    }
    return "!!";
}

string string_decode(string s)
{
    s.pop_back();
    s.erase(s.begin() + 0);
    return s;
}

void print_ip_host(struct addrinfo *res)
{
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *ser = (struct sockaddr_in *)res->ai_addr;
    void *addr = &(ser->sin_addr);
    inet_ntop(AF_INET, addr, ipstr, sizeof ipstr);

    cout << ipstr << endl;
}

int id_match(string *n_port, int n, string port)
{
    for (int i = 0; i < n; i++)
    {
        if (n_port[i] == port)
            return i;
    }
    return -1;
}

int sock_match(int *n_port, int n, int port)
{
    for (int i = 0; i < n; i++)
    {
        if (n_port[i] == port)
            return i;
    }
    return -1;
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

int send_filename(string fname, int sfd)
{
    fname = "F" + fname;
    fname.push_back(3);
    return send_msg(fname.c_str(), sfd);
}

int send_ack(string fname, int sfd, bool found)
{
    fname.push_back(3);
    if (found)
        fname = "A" + fname;
    else
        fname = "N" + fname;
    return send_msg(fname.c_str(), sfd);
}

string chararr_tostr(char *arr, int len)
{
    string s = "";
    for (int i = 0; i < len; i++)
        s.push_back(arr[i]);
    return s;
}

bool check_file(string msg, vector<string> files)
{
    string s = msg;
    for (int i = 0; i < files.size(); i++)
    {
        if (files[i] == s)
            return true;
    }
    return false;
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
    string c_uid[neigh];
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

    int sockserver, server, sockclient[neigh], socknew[neigh] = {0}, socknewf[neigh] = {0};
    string con_port[neigh];
    fd_set masterfds;
    int maxfd;
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
        // close(sockserver);
        cout << "Error(117): Reuse Failed\n";
    }

    if (bind(sockserver, res->ai_addr, res->ai_addrlen) == -1)
    {
        // close(sockserver);
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
    char msg[neigh][256];
    int rev_b[neigh] = {0};
    int net_rec = neigh;
    int net_ack = neigh * (search.size());
    string port_m = "P" + port;
    port_m.push_back(3);
    string uid_m = "U" + uid;
    uid_m.push_back(3);

    FD_SET(sockserver, &masterfds);
    maxfd = sockserver;

    map<string, int> ack_nack;
    for (auto i : search)
    {
        ack_nack[i] = 0;
    }
    int print_uid[neigh];
    int index[neigh];

    while (true)
    {
        if (!neigh)
            break;
        for (int i = 0; i < neigh; i++)
        {
            if (!(con_cl[i]) && !(connect(sockclient[i], cres[i]->ai_addr, cres[i]->ai_addrlen) == -1))
            {

                tmpc--;
                con_cl[i] = 1;
                // cout << "sending...... " << port_m << endl;
                if (send_msg(port_m.c_str(), sockclient[i]) == -1)
                {
                    cout << "Error(225): Sending serv port failed for index " << i << endl;
                    return 0;
                }
                if (send_msg(uid_m.c_str(), sockclient[i]) == -1)
                {
                    cout << "Error(225): Sending serv uid failed for index " << i << endl;
                    return 0;
                }

                for (auto f : search)
                {
                    if (send_filename(f, sockclient[i]) == -1)
                    {
                        cout << "Error(248): Sending filename failed for file " << i << endl;
                        return 0;
                    }
                }
                string eot = "";
                eot.push_back(4);
                if (send_msg(eot.c_str(), sockclient[i]) == -1)
                {
                    cout << "Error(255): Sending closing failed " << endl;
                    return 0;
                }
                // cout << "Connected " << i << endl;
            }
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        readfds = masterfds;

        char buffer[INET_ADDRSTRLEN];

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) == -1)
        {
            cout << "Error(224): select failed\n";
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
                cout << "Error(236): Accept failed\n";
                return 0;
            }
            // cout << "Connection accepted fd " << newfd << endl;

            socknew[--tmps] = newfd;
            FD_SET(newfd, &masterfds);
            maxfd = max(maxfd, newfd);
        }

        for (int i = 0; i < neigh; i++)
        {
            if (socknew[i] != 0 && FD_ISSET(socknew[i], &readfds))
            {
                int recd = recv(socknew[i], &msg[i][rev_b[i]], sizeof(char) * (256 - rev_b[i]), 0);
                // cout << "Received ran recd:" << recd << " socketfd:" << socknew[i] << endl;
                if (recd < 0)
                {
                    cout << "Error(300): Not yet connected on index " << i << endl;
                    return 0;
                }
                else if (recd == 0)
                {
                    // cout << "Msg: Connection closed on index " << i << endl;
                    FD_CLR(socknew[i], &masterfds);
                    // close(socknew[i]);
                }
                else
                {
                    rev_b[i] += recd;
                    // cout << "Msg: " << chararr_tostr(msg[i], rev_b[i]) << "; socketfd: " << socknew[i] << endl;
                    string cmd = msg_parse(msg[i], rev_b[i]);
                    while (cmd != "!!")
                    {
                        if (cmd[cmd.size() - 1] == 3)
                        {
                            string cmdt = string_decode(cmd);
                            // iix=cmdt.size()+2;
                            if (cmd[0] == 'F')
                            {
                                bool fnd = check_file(cmdt, files);
                                int cidx = sock_match(socknewf, neigh, socknew[i]);
                                if (cidx == -1)
                                {
                                    cout << "Error(371): Socket matching failed for socket " << socknew[i] << endl;
                                    return 0;
                                }
                                send_ack(cmdt, sockclient[cidx], fnd);
                            }
                            else if (cmd[0] == 'P')
                            {
                                string c_port = cmdt;
                                int idx = id_match(neigh_port, neigh, c_port);
                                index[i] = idx;
                                if (idx == -1)
                                {
                                    cout << "Error(366): Port match failed for port " << c_port << endl;
                                    return 0;
                                }
                                // cout << c_port << " c_port\n";
                                socknewf[idx] = socknew[i];
                                con_port[i] = c_port;
                            }
                            else if (cmd[0] == 'A')
                            {
                                // cout << "Found " << cmdt << " at " << c_uid[i] << " with MD5 0 at depth 1\n";
                                if (!ack_nack[cmdt])
                                    ack_nack[cmdt] = stoi(c_uid[i]);
                                else
                                    ack_nack[cmdt] = min(ack_nack[cmdt], stoi(c_uid[i]));
                                net_ack--;
                            }
                            else if (cmd[0] == 'N')
                            {
                                // cout << "Found " << cmdt << " at " << 0 << " with MD5 0 at depth 0\n";
                                net_ack--;
                            }
                            else if (cmd[0] == 'U')
                            {
                                c_uid[i] = cmdt;
                                print_uid[index[i]] = stoi(cmdt);
                                // cout << "unique id is " << c_uid[i] << "at index " << i << "\n";
                            }
                            else if (cmd[0] != 4)
                            {
                                cout << "Error(336): Wrong format received on index " << i << endl;
                                return 0;
                            }
                        }
                        else if (msg[i][0] == 4)
                            net_rec--;

                        shiftarr(msg[i], rev_b[i], cmd.size());
                        rev_b[i] -= cmd.size();
                        cmd = msg_parse(msg[i], rev_b[i]);
                    }
                }
            }
        }
        if (net_ack <= 0 && net_rec <= 0)
            break;
    }
    for (int i = 0; i < neigh; i++)
        cout << "Connected to " << neigh_id[i] << " with unique-ID " << print_uid[i] << " on port " << neigh_port[i] << endl;
    for (auto i : search)
    {
        if (ack_nack[i])
            cout << "Found " << i << " at " << ack_nack[i] << " with MD5 0 at depth 1\n";
        else
            cout << "Found " << i << " at " << ack_nack[i] << " with MD5 0 at depth 0\n";
    }

    // cout << "Closed on " << port << endl;
    for (int i = 0; i < neigh; i++)
    {
        close(socknew[i]);
        close(sockclient[i]);
    }
    return 0;
}
