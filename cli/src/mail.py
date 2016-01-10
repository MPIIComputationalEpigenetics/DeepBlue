#
#  mail.py
#  DeepBlue Epigenomic Data Server - CLI
#
#  Created by Felipe Albrecht
#  Copyright (c) 2016 Max Planck Institute for Informatics. All rights reserved.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

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
    text = text + "deepblue@mpi-inf.mpg.de\n"
    text = text + "http://deepblue.mpi-inf.mpg.de\n"

    send_email(subject, text, config.email_sender, user_email)
