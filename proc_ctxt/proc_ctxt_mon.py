import os

def get_context_switches(pid):
    """Gets voluntary and nonvoluntary context switch counts for a process and its threads."""

    def read_csw_from_file(filepath):
        """Helper function to read context switch counts from a status file."""
        vol = nonvol = 0
        try:
            with open(filepath, 'r') as f:
                for line in f:
                    if line.startswith('voluntary_ctxt_switches:'):
                        vol = int(line.split()[1])
                    elif line.startswith('nonvoluntary_ctxt_switches:'):
                        nonvol = int(line.split()[1])
        except FileNotFoundError:
            pass
        return vol, nonvol

    # Get context switches for the main process
    process_vol_csw, process_nonvol_csw = read_csw_from_file(f'/proc/{pid}/status')

    # Get context switches for each thread
    threads_csw = {}
    task_dir = f'/proc/{pid}/task'
    if os.path.isdir(task_dir):
        for tid in os.listdir(task_dir):
            thread_vol_csw, thread_nonvol_csw = read_csw_from_file(f'{task_dir}/{tid}/status')
            threads_csw[tid] = {
                'voluntary': thread_vol_csw,
                'nonvoluntary': thread_nonvol_csw
            }

    return {
        'process': {
            'voluntary': process_vol_csw,
            'nonvoluntary': process_nonvol_csw
        },
        'threads': threads_csw
    }

if __name__ == "__main__":
    pid = int(input("Enter PID to monitor: "))
    csw_data = get_context_switches(pid)

    print(f"Process {pid} Context Switches:")
    print(f"  Voluntary: {csw_data['process']['voluntary']}")
    print(f"  Nonvoluntary: {csw_data['process']['nonvoluntary']}")

    print("\nThread Context Switches:")
    for tid, csw in csw_data['threads'].items():
        print(f"  Thread {tid}:")
        print(f"    Voluntary: {csw['voluntary']}")
        print(f"    Nonvoluntary: {csw['nonvoluntary']}")