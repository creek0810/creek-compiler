import pytest
import os


@pytest.fixture(scope="session")
def compiler():
    os.system("make")