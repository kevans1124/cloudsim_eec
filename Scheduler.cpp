//
//  Scheduler.cpp
//  CloudSim
//
//  Created by ELMOOTAZBELLAH ELNOZAHY on 10/20/24.
//

#include "Scheduler.hpp"
#include <vector>

static bool migrating = false;
static int total_machines = 0;
vector<MachineId_t> off_machines;

void Scheduler::Init() {
    // Retrieve total number of available machines
    total_machines = Machine_GetTotal();
    SimOutput("Scheduler::Init(): Total number of machines is " + to_string(total_machines), 1);
    SimOutput("Scheduler::Init(): Initializing scheduler", 1);
    
    // Activate all machines initially
    for (int i = 0; i < total_machines; i++) {
        Machine_SetState(MachineId_t(i), S0);
        machines.push_back(MachineId_t(i));
    }
    SimOutput("Scheduler::Init(): scheduler initialized successfully", 1);
}

void Scheduler::MigrationComplete(Time_t time, VMId_t vm_id) {
    // Mark migration as complete
    migrating = false;
}

void Scheduler::NewTask(Time_t now, TaskId_t task_id) {
    // Retreive task details
    bool req_GPU = IsTaskGPUCapable(task_id);
    unsigned req_memory = GetTaskMemory(task_id);
    VMType_t req_vm = RequiredVMType(task_id);
    CPUType_t req_cpu = RequiredCPUType(task_id);

    // Check existing virtual machines for availability
    for (auto& vm : vms) {
        SimOutput("Inspecting VM " + to_string(vm), 1);
        VMInfo_t vm_info = VM_GetInfo(vm);
        MachineInfo_t m_info = Machine_GetInfo(vm_info.machine_id);
        // Validate resource availability and compatibility
        if (vm_info.active_tasks.size() < m_info.num_cpus && m_info.active_tasks <= m_info.num_cpus) {
            SimOutput("cores available", 1);
            if (vm_info.cpu == req_cpu && vm_info.vm_type == req_vm && (m_info.gpus == req_GPU || (m_info.gpus && !req_GPU))
            && ((m_info.memory_size - m_info.memory_used) >= req_memory + 8)) {
                SimOutput("Task assigned to existing VM", 1);
                VM_AddTask(vm, task_id, GetTaskInfo(task_id).priority);
                return;
            }
        }  
    }
    
    // If no existing VM is available, create a new VM
    VMId_t new_vm = VM_Create(req_vm, req_cpu);
    vms.push_back(new_vm);
    
    // Assign the new VM to an available machine
    for (auto& machine : machines) {
        MachineInfo_t m_info = Machine_GetInfo(machine);
        SimOutput("machine " + to_string(machine), 1);
        if (m_info.s_state == S0) {
        //finding the first machine that, has the matching cpu type, gpu, and the memory is big enough
            if (m_info.cpu == req_cpu && (m_info.gpus == req_GPU || (m_info.gpus && !req_GPU))
                    && ((m_info.memory_size - m_info.memory_used) >= (req_memory + 8))) {
                VM_Attach(new_vm, machine);
                VM_AddTask(new_vm, task_id, GetTaskInfo(task_id).priority);
                SimOutput("Scheduler::NewTask(): A new vm has been created on machine " + to_string(machine), 1);
                return;
            }
        }
    }

     // Power on an inactive machine if necessary
    for (MachineId_t machine : machines) {
        MachineInfo_t m_info = Machine_GetInfo(machine);
        if (m_info.s_state == S5) {
            if (m_info.cpu == req_cpu && (m_info.gpus == req_GPU || (m_info.gpus && !req_GPU)
                    && ((m_info.memory_size - m_info.memory_used) >= (req_memory + 8)))) {
                SimOutput("NewTask(): A new machine is being woken up to handle task", 1);
                Machine_SetState(machine, S0);
                VM_Attach(new_vm, machine);
                VM_AddTask(new_vm, task_id, GetTaskInfo(task_id).priority);
                SimOutput("Task " + to_string(task_id) + " assigned to new VM " + to_string(new_vm) + " on powered-up Machine " + to_string(machine), 2);
                return;
            }
        }
    }
    SimOutput("No suitable machine found", 1);
}


void Scheduler::PeriodicCheck(Time_t now) {
    // This method should be called from SchedulerCheck()
    // SchedulerCheck is called periodically by the simulator to allow you to monitor, make decisions, adjustments, etc.
    // Unlike the other invocations of the scheduler, this one doesn't report any specific event
    // Recommendation: Take advantage of this function to do some monitoring and adjustments as necessary
}

void Scheduler::Shutdown(Time_t time) {
    // Do your final reporting and bookkeeping here.
    // Report about the total energy consumed
    // Report about the SLA compliance
    // Shutdown everything to be tidy :-)

    for(auto & vm: vms) {
        VM_Shutdown(vm);
    }
    SimOutput("SimulationComplete(): Finished!", 4);
    SimOutput("SimulationComplete(): Time is " + to_string(time), 4);
}

void Scheduler::TaskComplete(Time_t now, TaskId_t task_id) {
    // Do any bookkeeping necessary for the data structures
    // Decide if a machine is to be turned off, slowed down, or VMs to be migrated according to your policy
    // This is an opportunity to make any adjustments to optimize performance/energy
    SimOutput("Scheduler::TaskComplete(): Task " + to_string(task_id) + " is complete at " + to_string(now), 4);
}

static Scheduler Scheduler;

void InitScheduler() {
    SimOutput("InitScheduler(): Initializing scheduler", 4);
    Scheduler.Init();
}

void HandleNewTask(Time_t time, TaskId_t task_id) {
    SimOutput("HandleNewTask(): Received new task " + to_string(task_id) + " at time " + to_string(time), 4);
    Scheduler.NewTask(time, task_id);
}

void HandleTaskCompletion(Time_t time, TaskId_t task_id) {
    SimOutput("HandleTaskCompletion(): Task " + to_string(task_id) + " completed at time " + to_string(time), 4);
    Scheduler.TaskComplete(time, task_id);
}

void MemoryWarning(Time_t time, MachineId_t machine_id) {
    // The simulator is alerting you that machine identified by machine_id is overcommitted
    SimOutput("MemoryWarning(): Overflow at " + to_string(machine_id) + " was detected at time " + to_string(time), 0);
}

void MigrationDone(Time_t time, VMId_t vm_id) {
    // The function is called on to alert you that migration is complete
    SimOutput("MigrationDone(): Migration of VM " + to_string(vm_id) + " was completed at time " + to_string(time), 4);
    Scheduler.MigrationComplete(time, vm_id);
    migrating = false;
}

void SchedulerCheck(Time_t time) {
    // This function is called periodically by the simulator, no specific event
}

void SimulationComplete(Time_t time) {
    // This function is called before the simulation terminates Add whatever you feel like.
    cout << "SLA violation report" << endl;
    cout << "SLA0: " << GetSLAReport(SLA0) << "%" << endl;
    cout << "SLA1: " << GetSLAReport(SLA1) << "%" << endl;
    cout << "SLA2: " << GetSLAReport(SLA2) << "%" << endl;     // SLA3 do not have SLA violation issues
    cout << "Total Energy " << Machine_GetClusterEnergy() << "KW-Hour" << endl;
    cout << "Simulation run finished in " << double(time)/1000000 << " seconds" << endl;
    SimOutput("SimulationComplete(): Simulation finished at time " + to_string(time), 4);
    
    Scheduler.Shutdown(time);
}

void SLAWarning(Time_t time, TaskId_t task_id) {
    
}

void StateChangeComplete(Time_t time, MachineId_t machine_id) {
    // Called in response to an earlier request to change the state of a machine
}
