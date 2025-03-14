#from jsonschema import validate, Draft202012Validator
from jsonschema import validate, Draft202012Validator, ValidationError, FormatChecker

import json

with open("schemas/AP_schema.json","r") as f:
    schemaAP = json.load(f)

with open("data/AP.json","r") as f:
    AP = json.load(f)

APvalidator = Draft202012Validator(schemaAP, format_checker=FormatChecker())

#print("schema",schemaAP)
#print("AP", AP)
print("validating AP")
APvalidator.validate(AP)


with open("schemas/env_schema.json","r") as f:
    schemaEnv = json.load(f)

with open("data/environments.json","r") as f:
    env = json.load(f)

Env_validator = Draft202012Validator(schemaEnv, format_checker=FormatChecker())
print("validating environments")
Env_validator.validate(env)

