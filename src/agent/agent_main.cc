// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: yanshiguang02@baidu.com

#include "agent_impl.h"

#include <sofa/pbrpc/pbrpc.h>

#include <stdio.h>
#include <signal.h>

extern std::string FLAGS_agent_port;
extern std::string FLAGS_agent_http_port;
extern std::string FLAGS_master_addr;
extern std::string FLAGS_agent_work_dir;
extern std::string FLAGS_container;
extern std::string FLAGS_cgroup_root;
extern std::string FLAGS_task_acct;
extern double FLAGS_cpu_num;
extern int64_t FLAGS_mem_gbytes;
extern int64_t FLAGS_mem_bytes;

static volatile bool s_quit = false;
static void SignalIntHandler(int /*sig*/) {
    s_quit = true;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        char s[1024];
        if (sscanf(argv[i], "--port=%s", s) == 1) {
            FLAGS_agent_port = s;
        } else if (sscanf(argv[i], "--master=%s", s) == 1) {
            FLAGS_master_addr = s;
        } else if (sscanf(argv[i], "--work_dir=%s", s) == 1) {
            FLAGS_agent_work_dir = s;
        } else if (sscanf(argv[i],"--cpu=%s",s) == 1) {
            FLAGS_cpu_num = atof(s);
        } else if (sscanf(argv[i],"--mem=%s",s) == 1) {
            FLAGS_mem_gbytes = atoi(s);
        } else if (sscanf(argv[i],"--container=%s",s) == 1) {
            FLAGS_container = s;
        } else if (sscanf(argv[i], "--cgroup_mount_path=%s", s) == 1) {
            FLAGS_cgroup_root = s; 
        } else if (sscanf(argv[i], "--user=%s", s) == 1) {
            FLAGS_task_acct = s; 
        } else if(sscanf(argv[i], "--http-port=%s", s) == 1) {
            FLAGS_agent_http_port = s;
        }
         
        else {
            fprintf(stderr, "Invalid flag '%s'\n", argv[i]);
            exit(1);
        }
    }

    FLAGS_mem_bytes = FLAGS_mem_gbytes*(1024*1024*1024);
    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    galaxy::AgentImpl* agent_service = new galaxy::AgentImpl();

    if (!rpc_server.RegisterService(agent_service)) {
        return EXIT_FAILURE;
    }

    std::string server_host = std::string("0.0.0.0:") + FLAGS_agent_port;
    if (!rpc_server.Start(server_host)) {
        return EXIT_FAILURE;
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);
    while (!s_quit) {
        sleep(1);
    }

    return EXIT_SUCCESS;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
