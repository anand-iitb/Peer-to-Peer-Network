#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int stoi1(string s)
{
int sz=s.size(),ans=0;
for(int i=0;i<sz;i++)
{
ans*=10;
ans+=(s[i]-'0');
}
return ans;
}

void print(string s)
{
for(int i=0;i<s.size();i++)
	cout<<s[i]<<"||";
cout<<"\n";
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
        if (msg[i] == 3 || msg[i] == 4 || msg[i] == 5 || msg[i] == 9 || msg[i] == 11)
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

int send_msgl(const char *msg,int len, int sfd)
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
    int id = stoi1(testcase[0]);
    string port = testcase[1];
    string uid = testcase[2];
    int neigh = stoi1(testcase[3]);
    int neigh_id[neigh];
    string neigh_port[neigh];
    string c_uid[neigh];
    for (int i = 0; i < neigh; i++)
    {
        neigh_id[i] = stoi1(testcase[4 + 2 * i]);
        neigh_port[i] = testcase[5 + 2 * i];
    }
    vector<string> search;
    int search_ka_size = stoi1(testcase[4 + 2 * neigh]);
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
    bool con_cl[neigh] = {0}, con_sv[neigh] = {0};
    struct sockaddr their_addr;
    char msg[neigh][256];
    int rev_b[neigh] = {0};
    string port_m = "P" + port;
    port_m.push_back(3);
    string uid_m = "U" + uid;
    uid_m.push_back(3);

    FD_SET(sockserver, &masterfds);
    maxfd = sockserver;

    int print_uid[neigh];
    int index[neigh];
    map<string, pair<int, string>> f_found;
    int f_done[neigh];

    for (int i = 0; i < neigh; i++)
        f_done[i] = 2 * neigh - 1;

    bool done_dol[neigh] = {0};
    int antim_breaking_var = neigh;
    vector<string> pen_msg(neigh, "");

    for (auto i : search)
    {
        f_found[i] = {-1, "0"};
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
                    string y = "";
                    y += "L";
                    y += x;
                    y.push_back(7);
                    y+="2";
                    y.push_back(8);
                    y += uid;
                    y.push_back(3);
                    // cout << "Y: " << y.c_str()[2+x.size()] << "\n PY: ";
                    // print(y);
                    file_list_apna += y;
                }
                file_list_apna.push_back(9);
                
                

                if (send_msgl(file_list_apna.c_str(), file_list_apna.size(),sockclient[i]) == -1)
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
                    int ix = sock_match(socknewf, neigh, socknew[i]);
                    //cout << "Msg: Connection closed on index " << neigh_port[ix] << endl;
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
                            else if (cmd[0] == 'U')
                            {
                                c_uid[i] = cmdt;
                                print_uid[index[i]] = stoi1(cmdt);
                                // cout << "unique id is " << c_uid[i] << "at index " << i << "\n";
                            }
                            else if (cmd[0] == 'L')
                            {
                                int fidx = cmd.find(7);
                                int hcidx = fidx + 2;
                                string fl_name = cmd.substr(1, fidx - 1);
                                string hc = cmd.substr(fidx + 1, 1);
                                int hc_int = stoi1(hc) - 1;
                                //less than 0 
                               // cout<<"host count is "<<hc<<"\n";
                                
                                string rec_uuid = cmd.substr(hcidx + 1, cmd.size() - hcidx - 2);
                                if(hc_int<0)
                                {
                                    cout << "Error(367): Hashcode less than 0 " << rec_uuid << ":UID " << hc_int << ":hop count " << fl_name << ":flname " << uid << ":my uid " << endl;
                                    // return 0;
                                }
                                bool found_f = 0;
                                           //cout<<"file_list_apna received in cmd ";
                    //print(cmd);
                                
                                for (int j = 0; j < search.size(); j++)
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
                                    }
                                    else if (f_found[fl_name].first > 2 - hc_int)
                                    {
                                        f_found[fl_name] = {2 - hc_int, rec_uuid};
                                    }
                                    else if (f_found[fl_name].first == 2 - hc_int && stoi1(f_found[fl_name].second) > stoi1(rec_uuid))
                                    {
                                        f_found[fl_name] = {2 - hc_int, rec_uuid};
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
                                            if (send_msgl(cmd.c_str(), cmd.size(),sockclient[j]) < 0)
                                            {
                                                cout << "Error(681): Forwarding filename failed for cmd ";
                                                for (int k = 0; k < cmd.size(); k++)
                                                {
                                                    cout << cmd[k] << " | ";
                                                }
                                                cout << endl;
                                                perror("46455555555");
                                            }
                                        }
                                        else
                                        {
                                            pen_msg[j] += cmd;
                                        }
                                    }
                                   // cout<<"hc_int is "<<hc_int<<"\n";

                                }
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

                        shiftarr(msg[i], rev_b[i], cmd.size());
                        rev_b[i] -= cmd.size();
                        cmd = msg_parse(msg[i], rev_b[i]);
                    }
                }
            }
        }

        if (antim_breaking_var <= 0)
        {
            break;
        }
    }
    for (int i = 0; i < neigh; i++)
        cout << "Connected to " << neigh_id[i] << " with unique-ID " << print_uid[i] << " on port " << neigh_port[i] << endl;
    for (auto i : search)
    {
        if (f_found[i].first > 0)
            cout << "Found " << i << " at " << f_found[i].second << " with MD5 0 at depth " << f_found[i].first << endl;
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
