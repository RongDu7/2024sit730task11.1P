import tkinter as tk
from tkinter import messagebox
import paho.mqtt.client as mqtt

# MQTT settings
broker = "mqtt-dashboard.com"
topic = "sensor/data"

# User data (for demonstration purposes, you might want to use a database)
users = {"admin": "admin"}

# GUI setup
root = tk.Tk()
root.title("Smart Flowerpot System")

# Frames for different views
login_frame = tk.Frame(root)
signup_frame = tk.Frame(root)
data_frame = tk.Frame(root)

# Variables for login and signup
username_var = tk.StringVar()
password_var = tk.StringVar()
new_username_var = tk.StringVar()
new_password_var = tk.StringVar()

# Variables to display data
temperature_var = tk.StringVar(value="Temperature: N/A")
humidity_var = tk.StringVar(value="Humidity: N/A")
light_var = tk.StringVar(value="Light Intensity: N/A")

def show_frame(frame):
    frame.tkraise()

for frame in (login_frame, signup_frame, data_frame):
    frame.grid(row=0, column=0, sticky='news')

def login():
    username = username_var.get()
    password = password_var.get()
    if username in users and users[username] == password:
        show_frame(data_frame)
    else:
        messagebox.showerror("Login failed", "Invalid username or password")

def signup():
    username = new_username_var.get()
    password = new_password_var.get()
    if username in users:
        messagebox.showerror("Signup failed", "Username already exists")
    else:
        users[username] = password
        messagebox.showinfo("Signup successful", "You can now log in")
        show_frame(login_frame)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(topic)

def on_message(client, userdata, msg):
    data = msg.payload.decode().split(',')
    temperature = data[0] if len(data) > 0 else "N/A"
    humidity = data[1] if len(data) > 1 else "N/A"
    light = data[2] if len(data) > 2 else "N/A"
   
    temperature_var.set(f"Temperature: {temperature} °C")
    humidity_var.set(f"Humidity: {humidity} %")
    light_var.set(f"Light Intensity: {light} lx")

# Login frame widgets
tk.Label(login_frame, text="Login").grid(row=0, column=1)
tk.Label(login_frame, text="Username").grid(row=1, column=0)
tk.Entry(login_frame, textvariable=username_var).grid(row=1, column=1)
tk.Label(login_frame, text="Password").grid(row=2, column=0)
tk.Entry(login_frame, textvariable=password_var, show="*").grid(row=2, column=1)
tk.Button(login_frame, text="Login", command=login).grid(row=3, column=1)
tk.Button(login_frame, text="Sign Up", command=lambda: show_frame(signup_frame)).grid(row=4, column=1)

# Signup frame widgets
tk.Label(signup_frame, text="Sign Up").grid(row=0, column=1)
tk.Label(signup_frame, text="Username").grid(row=1, column=0)
tk.Entry(signup_frame, textvariable=new_username_var).grid(row=1, column=1)
tk.Label(signup_frame, text="Password").grid(row=2, column=0)
tk.Entry(signup_frame, textvariable=new_password_var, show="*").grid(row=2, column=1)
tk.Button(signup_frame, text="Sign Up", command=signup).grid(row=3, column=1)
tk.Button(signup_frame, text="Back to Login", command=lambda: show_frame(login_frame)).grid(row=4, column=1)

# Data frame widgets
tk.Label(data_frame, textvariable=temperature_var, font=("Helvetica", 16)).pack(pady=10)
tk.Label(data_frame, textvariable=humidity_var, font=("Helvetica", 16)).pack(pady=10)
tk.Label(data_frame, textvariable=light_var, font=("Helvetica", 16)).pack(pady=10)

# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker, 1883, 60)
client.loop_start()

show_frame(login_frame)
root.mainloop()