#Makefile for PowerAnalysis-daemon

TARGET=PowerAnalysis-daemon

INCLUDE = ../../include
SOURCE = PowerAnalysis-daemon.c \
	$(INCLUDE)/cJSON.c \
	$(INCLUDE)/wrappers.c \
	$(INCLUDE)/scLogger.c 
FLAGS = -Wall -Wstrict-prototypes -Wwrite-strings -I$(INCLUDE) -I/usr/local/include -L/usr/local/lib -lm -lwiringPi
VPATH=$(INCLUDE)
OBJ=$(join $(addsuffix ../obj/, $(dir $(SOURCE))), $(notdir $(SOURCE:.c=.o))) 
DEPENDS=$(join $(addsuffix ../.dep/, $(dir $(SOURCE))), $(notdir $(SOURCE:.c=.d)))

all: $(TARGET)
	@true

## Rule for making the actual target
$(TARGET): $(OBJ)
	@echo "============="
	@echo "Linking the target $@"
	@echo "============="
	@gcc $(FLAGS) -o $@ $^ $(LIBS)
	@echo -- Link finished --

## Generic compilation rule
%.o : %.c
	@mkdir -p $(dir $@)
	@echo "============="
	@echo "Compiling $<"
	@gcc $(FLAGS) -c $< -o $@

## Rules for object files from c files
## Object file for each file is put in obj directory
## one level up from the actual source directory.
../obj/%.o : %.c
	@mkdir -p $(dir $@)
	@echo "============="
	@echo "Compiling $<"
	@gcc $(FLAGS) -c $< -o $@

# Rule for "other directory"  You will need one per "other" dir
$(INCLUDE)/../obj/%.o : %.c
	@mkdir -p $(dir $@)
	@echo "============="
	@echo "Compiling $<"
	@gcc $(FLAGS) -c $< -o $@

## Make dependancy rules
../.dep/%.d: %.cpp
	@mkdir -p $(dir $@)
	@echo "============="
	@echo Building dependencies file for $*.o
	@$(SHELL) -ec 'gcc -M $(FLAGS) $< | sed "s^$*.o^../obj/$*.o^" > $@'

## Dependency rule for "other" directory
$(INCLUDE)/../.dep/%.d: %.c
	@mkdir -p $(dir $@)
	@echo "============="
	@echo Building dependencies file for $*.o
	@$(SHELL) -ec 'gcc -M $(FLAGS) $< | sed "s^$*.o^$(INCLUDE)/../obj/$*.o^" > $@'

## Include the dependency files
-include $(DEPENDS)

testClient: $(INCLUDE)
	gcc -o PowerAnalysis-daemon-testClient PowerAnalysis-daemon-testClient.c $(FLAGS)

install:
	mv PowerAnalysis-daemon /usr/lib/SC-daemons/

clean:
	@-rm -f $(TARGET) $(OBJ) $(DEPENDS)
	@-rm -r ../obj
