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
        if (msg[i] == 3 || msg[i] == 4 || msg[i] == 5 || msg[i] == 9 || msg[i] == 11 || msg[i] == 15 || msg[i] == 16)
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

string chararr_tostr(char *arr, int len)
{
    string s = "";
    for (int i = 0; i < len; i++)
        s.push_back(arr[i]);
    return s;
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

int est_con(string ip, string port)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int con_sock = -1;

    if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &res) != 0)
    {
        // perror("Error(est_con)");
        return -1;
    }

    if (res == nullptr)
    {
        cout << "Error(res_con): res NULL\n";
        return -1;
    }

    if ((con_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        // perror("Error(est_con):");
        return -1;
    }

    while (true)
    {
        if (!(connect(con_sock, res->ai_addr, res->ai_addrlen) == -1))
        {
            return con_sock;
        }
    }
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
        // perror("opendir");s
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
    vector<string> c_uid(neigh, "");
    for (int i = 0; i < neigh; i++)
    {
        neigh_id[i] = stoi(testcase[4 + 2 * i]);
        neigh_port[i] = testcase[5 + 2 * i];
    }
    vector<string> search;
    int search_ka_size = stoi(testcase[4 + 2 * neigh]);
    int varrrr1 = testcase.size();
    for (int i = 5 + 2 * neigh; i < varrrr1; i++)
    {
        if (!search_ka_size)
            break;
        search.push_back(testcase[i]);
        search_ka_size--;
    }
    sort(search.begin(), search.end());

    /// testcases and files reading done///

    int sockserver, sockclient[neigh], socknewf[neigh] = {0};
    vector<int> socknew;
    // string con_port[neigh];
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
        // cout << "Connect: Port " << ((struct sockaddr_in *)&cres[i]->ai_addr)->sin_port << " Neigh " << i << endl;

        if ((sockclient[i] = socket(cres[i]->ai_family, cres[i]->ai_socktype, cres[i]->ai_protocol)) == -1)
        {
            cout << "Error(146): socket failed on client " << neigh_id[i] << endl;
            return 0;
        }
    }

    if (listen(sockserver, 1000*neigh) == -1)
    {
        cout << "Error(153): Listen failed\n";
    }

    int tmpc = neigh;
    bool con_cl[neigh] = {0};
    struct sockaddr their_addr;
    vector<char *> msg;
    vector<int> rev_b;
    string port_m = "P" + port;
    port_m.push_back(3);
    string uid_m = "U" + uid;
    uid_m.push_back(3);

    FD_SET(sockserver, &masterfds);
    maxfd = sockserver;

    int print_uid[neigh];
    vector<int> index(neigh, -1);
    map<string, pair<int, string>> f_found;
    map<string, pair<string, string>> f_iport;
    int f_done[neigh], reqf_done[neigh];
    bool done_dol[neigh] = {0}, fwd_done_req[neigh] = {0};
    int antim_breaking_var = neigh;
    vector<string> pen_msg(neigh, "");
    int kitni_file_mili = 0;
    bool dow_req_bhejdi = 0;
    int brkbrkbrk = neigh;

    for (int i = 0; i < neigh; i++)
    {
        f_done[i] = 2 * neigh - 1;
        reqf_done[i] = neigh - 1;
    }
    for (auto i : search)
    {
        f_found[i] = {-1, "0"};
        f_iport[i] = {"", ""};
    }

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

                string file_list_apna = "";
                for (auto x : files)
                {
                    file_list_apna += "L";
                    file_list_apna += x;
                    file_list_apna.push_back(7);
                    file_list_apna.push_back('2');
                    file_list_apna.push_back(7);
                    file_list_apna += uid;
                    file_list_apna.push_back(7);
                    file_list_apna += "127.0.0.1";
                    file_list_apna.push_back(7);
                    file_list_apna += port;
                    file_list_apna.push_back(3);
                }
                file_list_apna.push_back(9);

                if (send_msg(file_list_apna.c_str(), sockclient[i]) == -1)
                {
                    cout << "Error(225): Sending file list failed for index " << i << endl;
                    return 0;
                }
                for (int j = 0; j < neigh; j++)
                    f_done[j]--;
                for (int j = 0; j < neigh; j++)
                {
                    if (f_done[j] == 0 && !done_dol[j])
                    {
                        string cs = "";
                        cs.push_back(11);
                        send_msg(cs.c_str(), sockclient[j]);
                        done_dol[j] = 1;
                    }
                }
                if (pen_msg[i].size() > 0)
                {
                    if (send_msg(pen_msg[i].c_str(), sockclient[i]) == -1)
                    {
                        cout << "Error(225): sending pending msg for index " << i << endl;
                        return 0;
                    }
                }
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
            socknew.push_back(newfd);
            char *x_msg = new char[256];
            msg.push_back(x_msg);
            rev_b.push_back(0);
            FD_SET(newfd, &masterfds);
            maxfd = max(maxfd, newfd);
        }
        int socknew_ka_size = socknew.size();
        for (int i = 0; i < socknew_ka_size; i++)
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
                    // int ix = sock_match(socknewf, neigh, socknew[i]);
                    // cout << "Msg: Connection closed on index " << i << endl;
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

                            if (cmd[0] == 'P')
                            {
                                string c_port = cmdt;
                                int idx = id_match(neigh_port, neigh, c_port);
                                int indx_sz = index.size();
                                index.resize(max(socknew_ka_size, indx_sz), -1);
                                index[i] = idx;
                                if (idx == -1)
                                {
                                    cout << "Error(366): Port match failed for port " << c_port << endl;
                                    return 0;
                                }
                                // cout << c_port << " c_port\n";
                                socknewf[idx] = socknew[i];
                                // con_port[i] = c_port;
                            }
                            else if (cmd[0] == 'U')
                            {
                                c_uid.push_back(cmdt);
                                print_uid[index[i]] = stoi(cmdt);
                                // cout << "unique id is " << c_uid[i] << "at index " << i << "\n";
                            }
                            else if (cmd[0] == 'L')
                            {
                                int fidx = cmd.find(7);
                                int hcidx = fidx + 2;
                                int uidx = cmd.find(7, hcidx + 1);
                                int ipdx = cmd.find(7, uidx + 1);

                                string fl_name = cmd.substr(1, fidx - 1);
                                string hc = cmd.substr(fidx + 1, hcidx - fidx - 1);
                                int hc_int = stoi(hc) - 1;
                                string rec_uuid = cmd.substr(hcidx + 1, uidx - hcidx - 1);
                                string rec_ip = cmd.substr(uidx + 1, ipdx - uidx - 1);
                                string rec_port = cmd.substr(ipdx + 1, cmd.size() - ipdx - 2);
                                bool found_f = 0;
                                int search_kaaa_size = search.size();

                                for (int j = 0; j < search_kaaa_size; j++)
                                {
                                    if (fl_name == search[j])
                                    {
                                        found_f = 1;
                                        break;
                                    }
                                }
                                if (found_f)
                                {
                                    if (f_found[fl_name].first == -1)
                                    {
                                        f_found[fl_name] = {2 - hc_int, rec_uuid};
                                        f_iport[fl_name] = {rec_ip, rec_port};
                                        kitni_file_mili++;
                                    }
                                    else if (f_found[fl_name].first > 2 - hc_int)
                                    {
                                        f_found[fl_name] = {2 - hc_int, rec_uuid};
                                        f_iport[fl_name] = {rec_ip, rec_port};
                                    }
                                    else if (f_found[fl_name].first == 2 - hc_int && stoi(f_found[fl_name].second) > stoi(rec_uuid))
                                    {
                                        f_found[fl_name] = {2 - hc_int, rec_uuid};
                                        f_iport[fl_name] = {rec_ip, rec_port};
                                    }
                                }
                                if (hc_int > 0)
                                {
                                    int ex_idx = sock_match(socknewf, neigh, socknew[i]);
                                    cmd[fidx + 1] -= 1;
                                    for (int j = 0; j < neigh; j++)
                                    {
                                        if (j == ex_idx)
                                            continue;
                                        if (con_cl[j])
                                        {
                                            if (send_msg(cmd.c_str(), sockclient[j]) < 0)
                                            {
                                                cout << "Error(681): Forwarding filename failed for cmd " << cmd << endl;
                                            }
                                        }
                                        else
                                        {
                                            pen_msg[j] += cmd;
                                        }
                                    }
                                }
                            }
                            else if (cmd[0] == 'D')
                            {
                                int fidx = cmd.find(7);
                                int hcidx = fidx + 2;
                                int uidx = cmd.find(7, hcidx + 1);
                                int ipdx = cmd.find(7, uidx + 1);

                                string fl_name = cmd.substr(1, fidx - 1);
                                string hc = cmd.substr(fidx + 1, hcidx - fidx - 1);
                                // int hc_int = stoi(hc) - 1;
                                string rec_uuid = cmd.substr(hcidx + 1, uidx - hcidx - 1);
                                string rec_ip = cmd.substr(uidx + 1, ipdx - uidx - 1);
                                string rec_port = cmd.substr(ipdx + 1, cmd.size() - ipdx - 2);
                                // FD_CLR(socknew[i], &masterfds);
                                // close(socknew[i]);
                                int sock_sendto;
                                if (stoi(hc) == 2)
                                {
                                    sock_sendto = est_con(rec_ip, rec_port);
                                    if (sock_sendto < 0)
                                    {
                                        cout << "Error(559): Establishing connection failed for iport " << rec_ip << ":" << rec_port << endl;
                                        return 0;
                                    }
                                }
                                else if (stoi(hc) == 1)
                                {
                                    int idx_uid = sock_match(print_uid, neigh, stoi(rec_uuid));
                                    sock_sendto = sockclient[idx_uid];
                                }
                                else
                                {
                                    cout << "Error(570): Invalid hop count on download req, hc = " << hc << endl;
                                    return 0;
                                }

                                if (send_file(fl_name, sock_sendto, argv[2]) < 0)
                                {
                                    cout << "Error(498): Error in sending file " << cmdt << "\n";
                                    return 0;
                                }

                                if (stoi(hc) == 2)
                                {
                                    close(sock_sendto);
                                }
                            }
                            else if (cmd[0] == 'S')
                            {
                                pair<int, int> rrr = recv_file(socknew[i], msg[i], rev_b[i], down_dir_name);
                                if (rrr.second < 0)
                                {
                                    cout << "Error(515): Receiving failed\n";
                                    return 0;
                                }
                                cmd = "";
                                cmd.insert(0, rrr.second, 'r');
                                rev_b[i] = rrr.first;
                                kitni_file_mili--;
                                // FD_CLR(socknew[i], &masterfds);
                                // close(socknew[i]);
                            }
                            else if (cmd[0] != 4)
                            {
                                cout << "Error(336): Wrong format received on index " << i << endl;
                                return 0;
                            }
                        }
                        else if (msg[i][0] == 9)
                        {
                            int f_idx = sock_match(socknewf, neigh, socknew[i]);
                            for (int j = 0; j < neigh; j++)
                            {
                                if (j == f_idx)
                                    continue;
                                f_done[j]--;
                            }
                            for (int j = 0; j < neigh; j++)
                            {
                                if (j == f_idx)
                                    continue;
                                if (f_done[j] == 0 && !done_dol[j])
                                {
                                    string cs = "";
                                    cs.push_back(11);
                                    if (con_cl[j])
                                        send_msg(cs.c_str(), sockclient[j]);
                                    else
                                        pen_msg[j] += cs;
                                    done_dol[j] = 1;
                                }
                            }
                        }
                        else if (msg[i][0] == 11)
                        {
                            antim_breaking_var--;
                        }
                        else if (msg[i][0] == 15)
                        {
                            int sahi_idx = index[i];
                            for (int j = 0; j < neigh; j++)
                            {
                                if (sahi_idx == j)
                                    continue;
                                reqf_done[j]--;
                            }

                            for (int j = 0; j < neigh; j++)
                            {
                                if (reqf_done[j] == 0 && dow_req_bhejdi && !fwd_done_req[j])
                                {
                                    string bnd_kro = "";
                                    bnd_kro.push_back(16);
                                    send_msg(bnd_kro.c_str(), sockclient[j]);
                                    fwd_done_req[j] = 1;
                                }
                            }
                        }
                        else if (msg[i][0] == 16)
                        {
                            brkbrkbrk--;
                        }
                        shiftarr(msg[i], rev_b[i], cmd.size());
                        rev_b[i] -= cmd.size();
                        cmd = msg_parse(msg[i], rev_b[i]);
                    }
                }
            }
        }

        if (antim_breaking_var <= 0 && !dow_req_bhejdi)
        {
            // send_dow_req();
            int search1_ka_size = search.size();
            for (int j = 0; j < search1_ka_size; j++)
            {
                if (f_found[search[j]].first == -1)
                    continue;
                int send_to_sock = 0;
                if (f_found[search[j]].first == 2)
                {
                    send_to_sock = est_con(f_iport[search[j]].first, f_iport[search[j]].second);
                    if (send_to_sock < 0)
                    {
                        cout << "Error(597): Establishing connection failed for iport " << f_iport[search[j]].first << ":" << f_iport[search[j]].second << endl;
                        return 0;
                    }
                }
                else
                {
                    int idx_uid = sock_match(print_uid, neigh, stoi(f_found[search[j]].second));
                    send_to_sock = sockclient[idx_uid];
                }
                string req_of_dow = "D" + search[j];
                req_of_dow.push_back(7);
                req_of_dow.push_back('0' + f_found[search[j]].first);
                req_of_dow.push_back(7);
                req_of_dow += uid;
                req_of_dow.push_back(7);
                req_of_dow += "127.0.0.1";
                req_of_dow.push_back(7);
                req_of_dow += port;
                req_of_dow.push_back(3);
                send_msg(req_of_dow.c_str(), send_to_sock);
                if (f_found[search[j]].first == 2)
                {
                    close(send_to_sock);
                }
                // kitni_file_mili--;
            }
            dow_req_bhejdi = 1;
            for (int k = 0; k < neigh; k++)
            {
                if (reqf_done[k] == 0 && dow_req_bhejdi && !fwd_done_req[k])
                {
                    string bnd_kro = "";
                    bnd_kro.push_back(16);
                    send_msg(bnd_kro.c_str(), sockclient[k]);
                    fwd_done_req[k] = 1;
                }
            }
            for (int k = 0; k < neigh; k++)
            {
                string reb = "";
                reb.push_back(15);
                send_msg(reb.c_str(), sockclient[k]);
            }
        }

        if (antim_breaking_var <= 0 && dow_req_bhejdi && brkbrkbrk == 0 && kitni_file_mili == 0)
            break;
    }
    for (int i = 0; i < neigh; i++)
        cout << "Connected to " << neigh_id[i] << " with unique-ID " << print_uid[i] << " on port " << neigh_port[i] << endl;
    for (auto i : search)
    {
        if (f_found[i].first > 0)
        {
            string address = chararr_tostr(argv[2], length_of_char(argv[2])) + "Downloaded/" + i;
            cout << "Found " << i << " at " << f_found[i].second << " with MD5 " << file_to_md5(address) << " at depth " << f_found[i].first << endl;
        }
        else
            cout << "Found " << i << " at " << 0 << " with MD5 0 at depth 0\n";
    }
    // cout << "Closed on " << port << endl;
    for (int i = 0; i < neigh; i++)
    {
        close(socknew[i]);
        close(sockclient[i]);
    }
    return 0;
}
