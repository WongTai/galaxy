# -*- coding:utf-8 -*-
# Copyright (c) 2015, Galaxy Authors. All Rights Reserved
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Author: wangtaize@baidu.com
# Date: 2015-03-30
import logging
from django.views.decorators.csrf import csrf_exempt
from console.service import decorator as service_decorator
from common import http
from common import decorator as com_decorator
from console.service import helper
from bootstrap import settings
from galaxy import wrapper
from console.taskgroup import helper
LOG = logging.getLogger("console")
def list_service(request):
    """
    get current user's service list
    """
    builder = http.ResponseBuilder()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()

    client = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    status,jobs = client.list_jobs()
    LOG.info(status)
    if not status:
        return builder.error('fail to list jobs').build_json()
    ret = []
    for job in jobs:
        ret.append(job.__dict__)
    return builder.ok(data=ret).build_json()

@csrf_exempt
def create_service(request):
    """
    create a service
    """
    builder = http.ResponseBuilder()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    try:
        ret = helper.validate_init_service_group_req(request)
        LOG.info(ret)
        status,output = galaxy.create_task(ret['name'],ret['pkg_src'],
                                           ret['start_cmd'],
                                           ret['replicate_count'],
                                           ret['memory_limit']*1024*1024*1024,
                                           ret['cpu_share'],
                                           deploy_step_size = ret['deploy_step_size'])
        if not status:
            return builder.error('fail create task').build_json()
        return builder.ok().build_json()
    except Exception as e:
        return builder.error(str(e)).build_json()

def kill_service(request):
    builder = http.ResponseBuilder()
    id = request.GET.get('id',None)
    if not id:
        return builder.error('id is required').build_json()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()

    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    galaxy.kill_job(int(id))
    return builder.ok().build_json()

def update_service(request):
    builder = http.ResponseBuilder()
    id = request.GET.get('id',None)
    if not id:
        return builder.error('id is required').build_json()
    master_addr = request.GET.get('master',None)
    if not master_addr:
        return builder.error('master is required').build_json()
    replicate = request.GET.get('replicate',None)
    if not replicate:
        return builder.error('replicate is required').build_json()

    galaxy = wrapper.Galaxy(master_addr,settings.GALAXY_CLIENT_BIN)
    status = galaxy.update_job(id,replicate)
    if status:
        return builder.ok().build_json()
    else:
        return builder.error('fail to kill job').build_json()


