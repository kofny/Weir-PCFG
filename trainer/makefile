TARGET = venv_init requirements_init train
all: $(TARGET)

train: trainer/train.py
	cp $< .

venv_init:
	python3 -m venv venv

requirements_init: trainer/requirements.txt
	venv/bin/pip install -r $<
