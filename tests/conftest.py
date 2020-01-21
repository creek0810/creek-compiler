import os

import pytest


@pytest.fixture(scope="session")
def compiler():
    os.system("make")
