ifndef COLORS_MK

COLORS_MK := colors.mk

N := $(shell tput sgr0)
R := $(shell tput setaf 1)$(shell tput bold)
G := $(shell tput setaf 2)$(shell tput bold)
Y := $(shell tput setaf 3)$(shell tput bold)
B := $(shell tput setaf 4)$(shell tput bold)
P := $(shell tput setaf 5)$(shell tput bold)
C := $(shell tput setaf 6)$(shell tput bold)
W := $(shell tput setaf 7)$(shell tput bold)

endif
