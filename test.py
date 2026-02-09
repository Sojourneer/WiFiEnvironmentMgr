# ToDo invoke vallidate.py on env.get('PROJECT_DATA_DIR') folder
Import("env")

print("Hello world")

def before_buildfs(source, target, env):
    print("before_buildfs",env.get('PROJECT_DATA_DIR'))

def after_buildfs(source, target, env):
    print("after_buildfs",env.get('PROJECT_DATA_DIR'))

env.AddPreAction("buildfs", before_buildfs)
env.AddPostAction("buildfs", after_buildfs)
