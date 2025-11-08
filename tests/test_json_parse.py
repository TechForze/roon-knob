import json

payload = '''{"line1":"a","line2":"b","is_playing":false,"volume":33}'''
data = json.loads(payload)
assert data["line1"] == "a"
assert isinstance(data["volume"], int)
print("json parse ok")
