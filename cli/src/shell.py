from commands import get_command

class Shell:
    def __init__(self, context):
        self._context = context

    def loop(self):
        ok = True
        while ok is True:
            epidb = self._context.epidb
            line = raw_input(str(epidb.echo(self._context.user_key)) + " > ")
            cmd = line.strip()
            if len(cmd) == 0:
                continue
            command = get_command(cmd)
            if command is None:
                continue
            command(self._context)
