import mail


def create_user(context):
    okay = False
    while not okay:
        user_name = raw_input("User name: ")
        email = raw_input("Email: ")
        institution = raw_input("Institution: ")

        in_okay = raw_input('Are these informations correct? (Type Y for Yes)')

        if in_okay == 'Y':
            okay = True

        epidb = context.epidb
        (s, user_key) = epidb.add_user(user_name, email, institution,
                                       context.user_key)
        if s == 'okay':
            print('Okay.')
            print(user_key)
            mail.send_new_user_mail(user_name, user_key[1], email)
        else:
            print(user_key)


def list_users(context):
    epidb = context.epidb
    (status, users) = epidb.list_users(context.user_key)
    if status == "error":
        print status, users
        return
    for user in users:
        print user

commands_callback = {'create_user': create_user, 'list_users': list_users}


def get_command(name):
    if not name in commands_callback:
        print 'Invalid command ', name
        return None
    return commands_callback[name]
