import schedule
import time

def job():
    send_email_report()

# Schedule the job every day at a specific time
schedule.every().day.at("09:00").do(job)

while True:
    schedule.run_pending()
    time.sleep(1)
