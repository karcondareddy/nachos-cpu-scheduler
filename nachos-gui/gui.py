import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext, messagebox
import subprocess
import re
import os

# Paths (adjust if your NachOS installation is different)
NACHOS_DIR = "/home/avinash/nachos-3.4/code/threads"
CONFIG_PATH = "/home/avinash/nachos-3.4/code/config.txt"
PROCESSES_PATH = "/home/avinash/nachos-3.4/code/processes.txt"

class NachosGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("NachOS Multi-Algorithm Scheduler Studio")
        self.root.geometry("950x800")
        
        # Configure style
        style = ttk.Style()
        style.theme_use('clam')

        # --- TOP FRAME: INPUTS ---
        input_frame = tk.Frame(root)
        input_frame.pack(pady=10, fill=tk.X, padx=20)

        # Algorithm Selection
        tk.Label(input_frame, text="Algorithm:", font=("Arial", 10, "bold")).grid(row=0, column=0, padx=5, pady=5, sticky="e")
        self.algo_var = tk.StringVar()
        self.algo_dropdown = ttk.Combobox(input_frame, textvariable=self.algo_var, state="readonly", width=20)
        self.algo_dropdown['values'] = ("Round Robin", "HRRN", "Lottery", "Priority", "Multi-Level Queue")
        self.algo_dropdown.current(4) # Default to MLQ
        self.algo_dropdown.grid(row=0, column=1, padx=5, pady=5, sticky="w")

        # Config parameters
        tk.Label(input_frame, text="RR Quantum:").grid(row=0, column=2, padx=5, pady=5, sticky="e")
        self.quantum_entry = tk.Entry(input_frame, width=8)
        self.quantum_entry.insert(0, "3")
        self.quantum_entry.grid(row=0, column=3, padx=5, pady=5, sticky="w")

        tk.Label(input_frame, text="Max RR Quanta:").grid(row=0, column=4, padx=5, pady=5, sticky="e")
        self.max_quanta_entry = tk.Entry(input_frame, width=8)
        self.max_quanta_entry.insert(0, "3")
        self.max_quanta_entry.grid(row=0, column=5, padx=5, pady=5, sticky="w")

        # Process file / text editor
        tk.Label(input_frame, text="Processes (ID Burst Arrival [Priority]):", font=("Arial", 10, "bold")).grid(row=1, column=0, columnspan=6, pady=(15, 0), sticky="w")
        
        self.processes_text = scrolledtext.ScrolledText(input_frame, width=90, height=7, font=("Courier", 10))
        self.processes_text.grid(row=2, column=0, columnspan=6, pady=5)
        self.processes_text.insert(tk.END, "P1 12 0 4\nP2 2 2 2\nP3 1 3 1\nP4 6 8 5\nP5 3 10 3")

        # Action Buttons
        btn_frame = tk.Frame(input_frame)
        btn_frame.grid(row=3, column=0, columnspan=6, pady=5)
        tk.Button(btn_frame, text="Load File", command=self.select_file, width=15).pack(side=tk.LEFT, padx=10)
        tk.Button(btn_frame, text="Run Simulation", command=self.run_nachos, bg="green", fg="white", font=("Arial", 10, "bold"), width=20).pack(side=tk.LEFT, padx=10)

        # --- BOTTOM FRAME: TABS (Console, Table, Gantt) ---
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)

        # Tab 1: Console
        self.tab_console = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_console, text="Console Output")
        self.output_area = scrolledtext.ScrolledText(self.tab_console, wrap=tk.WORD, font=("Courier", 10))
        self.output_area.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Tab 2: Metrics Table
        self.tab_table = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_table, text="Metrics Table")
        
        columns = ("PID", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting")
        self.tree = ttk.Treeview(self.tab_table, columns=columns, show="headings", height=10)
        for col in columns:
            self.tree.heading(col, text=col)
            self.tree.column(col, width=100, anchor=tk.CENTER)
        self.tree.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.averages_label = tk.Label(self.tab_table, text="", font=("Arial", 11, "bold"))
        self.averages_label.pack(pady=10)

        # Tab 3: Gantt Chart
        self.tab_gantt = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_gantt, text="Gantt Chart")
        
        # Add a scrollbar to the canvas for long execution times
        self.canvas_frame = tk.Frame(self.tab_gantt)
        self.canvas_frame.pack(fill=tk.BOTH, expand=True)
        self.h_scroll = tk.Scrollbar(self.canvas_frame, orient=tk.HORIZONTAL)
        self.h_scroll.pack(side=tk.BOTTOM, fill=tk.X)
        self.canvas = tk.Canvas(self.canvas_frame, height=200, bg="white", xscrollcommand=self.h_scroll.set)
        self.canvas.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.h_scroll.config(command=self.canvas.xview)

    def select_file(self):
        filename = filedialog.askopenfilename(title="Select processes.txt", filetypes=[("Text files", "*.txt")])
        if filename:
            with open(filename, "r") as f:
                content = f.read()
            self.processes_text.delete(1.0, tk.END)
            self.processes_text.insert(tk.END, content)

    def run_nachos(self):
        # 1. Prepare Configuration
        algo_map = {"Round Robin": "RR", "HRRN": "HRRN", "Lottery": "Lottery", "Priority": "Priority", "Multi-Level Queue": "MLQ"}
        selected_flag = algo_map[self.algo_var.get()]

        try:
            quantum = int(self.quantum_entry.get())
            max_quanta = int(self.max_quanta_entry.get())
            with open(CONFIG_PATH, "w") as f:
                f.write(f"{quantum} {max_quanta} 4") # The 4 doesn't matter anymore, but we write it to prevent sscanf errors
        except Exception as e:
            messagebox.showerror("Error", f"Invalid Config: {e}")
            return

        # 2. Prepare Process Data
        processes_content = self.processes_text.get(1.0, tk.END).strip()
        if not processes_content: return
        
        process_data = {} # { 'P1': {'AT': 0, 'BT': 10, 'Prio': 4, 'CT': 0} }
        for line in processes_content.splitlines():
            parts = line.strip().split()
            if len(parts) >= 3:
                pid, bt, at = parts[0], int(parts[1]), int(parts[2])
                prio = int(parts[3]) if len(parts) > 3 else 0
                process_data[pid] = {'AT': at, 'BT': bt, 'Prio': prio, 'CT': 0}
        
        try:
            with open(PROCESSES_PATH, "w") as f:
                f.write(processes_content)
        except Exception as e:
            messagebox.showerror("Error", f"Cannot write processes file: {e}")
            return

        # 3. Execute Nachos
        self.output_area.delete(1.0, tk.END)
        self.root.update()

        try:
            result = subprocess.run(["./nachos", "-sch", selected_flag], cwd=NACHOS_DIR, capture_output=True, text=True, timeout=10)
            stdout = result.stdout
            self.output_area.insert(tk.END, stdout)
            
            # Switch to console tab
            self.notebook.select(self.tab_console)
            
            # 4. Parse Output for Gantt Chart & Table
            self.parse_and_visualize(stdout, process_data)

        except subprocess.TimeoutExpired:
            self.output_area.insert(tk.END, "\nERROR: Timeout.\n")
        except Exception as e:
            self.output_area.insert(tk.END, f"\nERROR: {e}\n")

    def parse_and_visualize(self, stdout, process_data):
        # Regex to capture execution blocks: e.g., "Time 0-3: P1 executes" or "Time 15-21: FCFS runs P1"
        # Group 1: Start time, Group 2: End time, Group 3: Process ID
        pattern = r"Time (\d+)-(\d+): (?:FCFS runs )?([A-Za-z0-9]+)"
        gantt_data = []
        max_time = 0

        for line in stdout.splitlines():
            m = re.search(pattern, line)
            if m:
                start, end, pid = int(m.group(1)), int(m.group(2)), m.group(3)
                gantt_data.append((start, end, pid))
                if end > max_time: max_time = end
                
                # The completion time is simply the final end time we see for a process
                if pid in process_data:
                    process_data[pid]['CT'] = max(process_data[pid]['CT'], end)

        # Update Table and Averages
        for item in self.tree.get_children():
            self.tree.delete(item)

        total_tat = 0
        total_wt = 0
        count = len(process_data)

        for pid, data in process_data.items():
            at = data['AT']
            bt = data['BT']
            ct = data['CT']
            prio = data['Prio']
            
            tat = ct - at
            wt = tat - bt
            
            total_tat += tat
            total_wt += wt

            self.tree.insert("", tk.END, values=(pid, at, bt, prio, ct, tat, wt))

        if count > 0:
            avg_tat = total_tat / count
            avg_wt = total_wt / count
            self.averages_label.config(text=f"Average Turnaround Time: {avg_tat:.2f} ticks   |   Average Waiting Time: {avg_wt:.2f} ticks")

        # Draw Gantt Chart
        self.draw_gantt(gantt_data, max_time)

    def draw_gantt(self, gantt_data, max_time):
        self.canvas.delete("all")
        if not gantt_data: return

        # Configuration
        rect_height = 50
        y_pos = 50
        pixels_per_tick = 30 # Scale factor for width
        pad_x = 20
        
        total_width = pad_x * 2 + max_time * pixels_per_tick
        self.canvas.config(scrollregion=(0, 0, total_width, 200))

        colors = ["#FF9999", "#99CCFF", "#99FF99", "#FFCC99", "#DDA0DD", "#FFD700", "#FFB6C1", "#87CEFA"]
        color_map = {}
        color_idx = 0

        for start, end, pid in gantt_data:
            if pid not in color_map:
                color_map[pid] = colors[color_idx % len(colors)]
                color_idx += 1

            x1 = pad_x + start * pixels_per_tick
            x2 = pad_x + end * pixels_per_tick
            
            # Draw block
            self.canvas.create_rectangle(x1, y_pos, x2, y_pos + rect_height, fill=color_map[pid], outline="black", width=2)
            
            # Draw Process ID
            self.canvas.create_text((x1 + x2) / 2, y_pos + rect_height / 2, text=pid, font=("Arial", 12, "bold"))
            
            # Draw ticks
            self.canvas.create_text(x1, y_pos + rect_height + 15, text=str(start), font=("Arial", 10))
            self.canvas.create_text(x2, y_pos + rect_height + 15, text=str(end), font=("Arial", 10))

if __name__ == "__main__":
    root = tk.Tk()
    app = NachosGUI(root)
    root.mainloop()
