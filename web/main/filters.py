"""Filters for Jinja templates"""
from datetime import datetime


def timestamp_to_date(timestamp):
    date = datetime.fromtimestamp(int(timestamp))
    return date.strftime('%Y-%m-%d %H:%M:%S')

# These filters will be registered in the application
filters = dict(
    timestamp_to_date=timestamp_to_date
)
