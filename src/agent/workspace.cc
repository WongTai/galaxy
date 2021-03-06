// Copyright (c) 2015, Galaxy Authors. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Author: wangtaize@baidu.com

#include "agent/workspace.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include "common/logging.h"
#include "agent/utils.h"

extern std::string FLAGS_task_acct;

namespace galaxy {

int DefaultWorkspace::Create() {
    LOG(INFO, "create workspace for task %d", m_task_info.task_id());
    if (m_has_created) {
        return 0;
    }
    //TODO safe path join
    //create work dir
    std::stringstream private_path;
    private_path << m_root_path;
    int status = 0;
    private_path << FLAGS_task_acct;
    status = MakePath(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    m_task_root_path = private_path.str();
    if (0 != status) {
        LOG(WARNING, "create task root path failed %s err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
        return status;
    }

    private_path << "/" << m_task_info.task_id();
    status = MakePath(private_path.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (status == 0) {
        m_task_root_path = private_path.str();
        m_has_created = true;
    } else {
        LOG(WARNING, "create task root path failed %s err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
        return status;
    }
    return status;
}

int DefaultWorkspace::MakePath(const char *path, mode_t mode)
{
    struct stat st;
    memset(&st, 0, sizeof(st));
    int status = 0;
    if (stat(path, &st) != 0) {
        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            status = -1;
        }
    }
    else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }
    return status;
}

std::string DefaultWorkspace::GetPath() {
    return m_task_root_path;
}

int DefaultWorkspace::Clean() {
    LOG(INFO,"clean task %d workspace",m_task_info.task_id());
    if (!m_has_created) {
        return 0;
    }
    struct stat st;
    int ret = 0;
    if (stat(m_task_root_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if (!file::Remove(m_task_root_path)) {
            LOG(WARNING, "clean task %d workspace %s failed", 
                    m_task_info.task_id(), 
                    m_task_root_path.c_str());
            ret = -1;
        } else {
            LOG(INFO, "clean task %d success", m_task_info.task_id());    
            ret = 0;
        }
    }
    return ret;
}
bool DefaultWorkspace::IsExpire() {
    return false;
}

TaskInfo DefaultWorkspace::GetTaskInfo() {
    return m_task_info;
}

int DefaultWorkspace::MoveTo(const std::string& new_dir) {
    if (access(new_dir.c_str(), F_OK) != 0) {
        LOG(WARNING, "access path %s failed",
                new_dir.c_str());
        return -1; 
    }

    std::string time_str;
    GetStrFTime(&time_str);
    std::stringstream ss;
    ss << new_dir << "/" << m_task_info.task_id() << "_" << time_str;
    std::string new_task_root_path = ss.str();
    if (access(new_task_root_path.c_str(), F_OK) == 0) {
        LOG(FATAL, "path %s already exists", 
                new_task_root_path.c_str()); 
        return -1;
    }
    
    int ret = rename(m_task_root_path.c_str(), new_task_root_path.c_str());
    if (ret == -1) {
        LOG(WARNING, "rename %s failed err[%d: %s]",
                m_task_root_path.c_str(), errno, strerror(errno));
        return -1;
    }
    m_task_root_path = new_task_root_path;
    return 0;
}

}



