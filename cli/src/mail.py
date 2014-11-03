import smtplib

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

from config import Config


def send_email(subject, text, from_, to):
    config = Config()
    msg = MIMEMultipart('alternative')
    msg['Subject'] = subject
    msg['From'] = from_
    msg['To'] = to

    part1 = MIMEText(text, 'plain')

    msg.attach(part1)

    s = smtplib.SMTP_SSL(config.email_server, config.email_port)
    s.login(config.email_user, config.email_password)
    s.sendmail(from_, to, msg.as_string())
    s.quit()


def send_new_user_mail(user, key, user_email):
    config = Config()
    subject = "DeepBlue Account"
    text = 'Hello ' + user + ",\n"
    text = text + "thanks for your registration!\n\n"
    text = text + "Your user key is '" + key + "' .\n"
    text = text + "This key is particular and private."
    text = text + " Please, do not share it.\n\n"
    text = text + "Do not hesitate to contact us if you have any question.\n\n"
    text = text + "DeepBlue Team\n"
    text = text + "deepblue@mpi-inf.mpg.de"

    send_email(subject, text, config.email_sender, user_email)
