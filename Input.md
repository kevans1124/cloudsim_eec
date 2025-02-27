machine class:
{
        # test with 16 x86 machines
        Number of machines: 16
        CPU type: X86
        Number of cores: 8
        Memory: 16384
        S-States: [120, 100, 100, 80, 40, 10, 0]
        P-States: [12, 8, 6, 4]
        C-States: [12, 3, 1, 0]
        MIPS: [1000, 800, 600, 400]
        GPUs: yes
}

machine class:
{
        # test with 24 arm machines
        Number of machines: 24
        Number of cores: 16
        CPU type: ARM
        Memory: 16384
        S-States: [120, 100, 100, 80, 40, 10, 0]
        P-States: [12, 8, 6, 4]
        C-States: [12, 3, 1, 0]
        MIPS: [1000, 800, 600, 400]
        GPUs: yes
}

machine class:
{
        # test with RISCV machines
        Number of machines: 8
        CPU type: RISCV
        Number of cores: 4
        Memory: 8192
        S-States: [100, 80, 60, 40, 20, 10, 0]
        P-States: [10, 7, 5, 3]
        C-States: [10, 2, 1, 0]
        MIPS: [800, 600, 400, 200]
        GPUs: no
}

task class:
{
        # base test with SLA0
        Start time: 60000
        End time : 800000
        Inter arrival: 6000
        Expected runtime: 2000000
        Memory: 8
        VM type: LINUX
        GPU enabled: no
        SLA type: SLA0
        CPU type: X86
        Task type: WEB
        Seed: 520230
}

task class:
{       
        # base test with SLA1
        Start time: 10000
        End time : 500000
        Inter arrival: 5000
        Expected runtime: 500000
        Memory: 4
        VM type: LINUX
        GPU enabled: no
        SLA type: SLA1
        CPU type: X86
        Task type: WEB
        Seed: 123456
}

task class:
{
        # simulating a load spike
        Start time: 300000
        End time: 310000
        Inter arrival: 500    
        Expected runtime: 1000000
        Memory: 8
        VM type: LINUX
        GPU enabled: yes     
        SLA type: SLA0
        CPU type: X86
        Task type: AI
        Seed: 987655
}

