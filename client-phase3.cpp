#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/md5.h>

using namespace std;

#define BUFFERSIZE 16384

string file_to_md5(const string &fname)
{

    char buffer[BUFFERSIZE];
    unsigned char digest[MD5_DIGEST_LENGTH];
    stringstream ss;
    string ans;
    ifstream ifs(fname, ifstream::binary);
    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    while (ifs.good())
    {
        ifs.read(buffer, BUFFERSIZE);
        MD5_Update(&md5Context, buffer, ifs.gcount());
    }
    ifs.close();
    int res = MD5_Final(digest, &md5Context);
    if (res == 0)
        return {};
    ss << hex << setfill('0');
    for (unsigned char uc : digest)
        ss << std::setw(2) << (int)uc;
    ans = ss.str();
    return ans;
}

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
        if (msg[i] == 3 || msg[i] == 4 || msg[i] == 5)
            return cmd;
    }
    return "!!";
}

int writetofile(FILE *doc, int wr_size, char *buf)
{
    int wr_done = 0;
    while (wr_done < wr_size)
    {
        int cur_wr = fwrite(buf, sizeof(char), wr_size - wr_done, doc);
        if (cur_wr < 0)
        {
            cout << "Error(writetofile): Error in writing file\n";
            return -1;
        }
        wr_done += cur_wr;
    }

    return wr_done;
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

int send_exe(const char *msg, int len, int sfd)
{
    int b_sent = 0;
    while (b_sent != len)
    {

        int p_sent = send(sfd, &msg[b_sent], len - b_sent, 0);
        if (p_sent == -1)
            return -1;
        b_sent += p_sent;
    }
    return b_sent;
}

pair<int, int> recv_file(int sock, char *buf, int rec_b, string dir)
{
    string cmd = msg_parse(buf, rec_b);
    string size, fname;
    int split = cmd.find(6);
    size = cmd.substr(1, split - 1);
    fname = cmd.substr(split + 2, cmd.size() - split - 3);
    int sizef = stoi(size);

    FILE *doc;
    string addr = dir + "/" + fname;
    doc = fopen(addr.c_str(), "wb");
    shiftarr(buf, rec_b, cmd.size());
    rec_b -= cmd.size();
    if (rec_b == 0)
    {
        rec_b = recv(sock, buf, 256 * sizeof(char), 0);
        if (rec_b == 0)
        {
            cout << "Error(recv_file): Connection closed on socket " << sock << endl;
        }
        else if (rec_b < 0)
        {
            cout << "Error(recv_file): Receive failed on socket " << sock << endl;
            return {-1, -1};
        }
    }

    int wr_done = 0;
    wr_done += writetofile(doc, rec_b, buf);
    if (wr_done < 0)
    {
        cout << "Error(recv_file): Error in writing file " << fname << endl;
        return {-1, -1};
    }

    while (true)
    {
        int recd = recv(sock, buf, 256 * sizeof(char), 0);

        if (recd == 0)
        {
            cout << "Error(recv_file): Connection closed on socket " << sock << endl;
        }
        else if (recd < 0)
        {
            cout << "Error(recv_file): Receive failed on socket " << sock << endl;
            return {-1, -1};
        }
        else
        {
            if (wr_done + recd >= sizef)
            {
                if (writetofile(doc, sizef - wr_done, buf) < 0)
                {
                    cout << "Error(recv_file): Writing to file failed for last segment for file " << fname << endl;
                    return {-1, -1};
                }
                fclose(doc);
                return {recd, sizef - wr_done};
            }
            else
            {
                if (writetofile(doc, recd, buf) < 0)
                {
                    cout << "Error(recv_file): Writing to file failed for some segment for file " << fname << endl;
                    return {-1, -1};
                }
                wr_done += recd;
            }
        }
    }
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
    int file11_ka_size=files.size();
    for (int i = 0; i < file11_ka_size; i++)
    {
        if (files[i] == s)
            return true;
    }
    return false;
}

int length_of_char(char *msg)
{
    int i = 0;
    while (msg[i] != '\0')
    {
        i++;
    }
    return i;
}

int send_file(string filename, int sock, char *dir)
{
    FILE *doc;
    string address = chararr_tostr(dir, length_of_char(dir)) + (dir[length_of_char(dir) - 1] != '/' ? "/" : "") + filename;
    doc = fopen(address.c_str(), "rb");
    fseek(doc, 0, SEEK_END);
    int size = ftell(doc);
    string s = "S" + to_string(size);
    s.push_back(6);
    s += "T" + filename;
    s.push_back(3);
    // s += "G";
    if (send_msg(s.c_str(), sock) == -1)
        return -1;

    char Sbuf[1024];
    fseek(doc, 0, SEEK_SET);
    int b_sent = 0;
    while (!feof(doc))
    {
        int n = fread(Sbuf, sizeof(char), 1024, doc);
        if (n > 0)
        {
            if (send_exe(Sbuf, n, sock) < 0)
            {
                return -1;
            }
            b_sent += n;
        }
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

    string down_dir_name = chararr_tostr(argv[2], length_of_char(argv[2])) + "Downloaded";
    mkdir(down_dir_name.c_str(), 0777);

    ifstream File;
    File.open(argv[1]);

    string line;
    vector<string> testcase;

    while (File.good())
    {
        File >> line;
        testcase.push_back(line);
    }
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
    int ttteeekkk=testcase.size();
    for (int i = 5 + 2 * neigh; i < ttteeekkk; i++)
    {
        if (!search_ka_size)
            break;
        search.push_back(testcase[i]);
        search_ka_size--;
    }
    sort(search.begin(), search.end());
    /// testcases and files reading done///

    int sockserver, sockclient[neigh], socknew[neigh] = {0}, socknewf[neigh] = {0};
    string con_port[neigh];
    fd_set masterfds;
    int maxfd;
    struct addrinfo hints, *res;
    struct addrinfo *cres[neigh];
    bool file_aagya[search.size()] = {0};

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
    bool con_cl[neigh] = {0};
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
    int sabki_request = neigh;
    int meri_requet = 0;
    bool req_bhejdi = 0;
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
                    FD_CLR(socknew[i], &masterfds);
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
                            else if (cmd[0] == 'D')
                            {
                             
                                int cidx = sock_match(socknewf, neigh, socknew[i]);
                                if (cidx == -1)
                                {
                                    cout << "Error(371): Socket matching failed for socket " << socknew[i] << endl;
                                    return 0;
                                }
                                if (send_file(cmdt, sockclient[cidx], argv[2]) < 0)
                                {
                                    cout << "Error(498): Error in sending file " << cmdt << "\n";
                                    return 0;
                                }
                            }
                            else if (cmd[0] == 'S')
                            {
                                
                                int cidx = sock_match(socknewf, neigh, socknew[i]);
                                if (cidx == -1)
                                {
                                    cout << "Error(371): Socket matching failed for socket " << socknew[i] << endl;
                                    return 0;
                                }
                                pair<int, int> rrr = recv_file(socknew[i], msg[i], rev_b[i], down_dir_name);
                                if (rrr.second < 0)
                                {
                                    cout << "Error(515): Receiving failed\n";
                                    return 0;
                                }
                                cmd = "";
                                cmd.insert(0, rrr.second, 'r');
                                rev_b[i] = rrr.first;
                                meri_requet--;
                            }
                            else if (cmd[0] != 4)
                            {
                                cout << "Error(336): Wrong format received on index " << i << endl;
                                return 0;
                            }
                        }
                        else if (msg[i][0] == 4)
                            net_rec--;
                        else if (msg[i][0] == 5)
                            sabki_request--;

                        shiftarr(msg[i], rev_b[i], cmd.size());
                        rev_b[i] -= cmd.size();
                        cmd = msg_parse(msg[i], rev_b[i]);
                    }
                }
            }
        }
        if (net_ack <= 0 && net_rec <= 0 && !req_bhejdi)
        {
            string dow_req = "";

            for (auto i : search)
            {
                string x = i;
                auto it = find(search.begin(), search.end(), x);
                int index = it - search.begin();
                if (!(ack_nack[i]))
                    continue;
                if (ack_nack[i] && !(file_aagya[index]))
                {
                    int temp = ack_nack[i];
                    int iii = sock_match(print_uid, neigh, temp); // getting the port;
                    if (iii < 0)
                    {
                        cout << "Error(448): wrong port info\n";
                        return 0;
                    }
                    dow_req.append("D" + i);
                    dow_req.push_back(3);
                    send_msg(dow_req.c_str(), sockclient[iii]);
                    meri_requet++;
                    file_aagya[index] = 1;
                    dow_req = "";
                }
            }
            for (int i = 0; i < neigh; i++)
            {
                string ttt = "";
                ttt.push_back(5);
                send_msg(ttt.c_str(), sockclient[i]);
            }
            req_bhejdi = 1;
        }
        if (net_ack <= 0 && net_rec <= 0 && sabki_request <= 0 && meri_requet <= 0)
        {
            break;
        }
    }
    for (int i = 0; i < neigh; i++)
        cout << "Connected to " << neigh_id[i] << " with unique-ID " << print_uid[i] << " on port " << neigh_port[i] << endl;
    for (auto i : search)
    {
        if (ack_nack[i])
        {
            string address = chararr_tostr(argv[2], length_of_char(argv[2])) + "Downloaded/" + i;
            cout << "Found " << i << " at " << ack_nack[i] << " with MD5 " << file_to_md5(address) << " at depth 1\n";
        }
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
