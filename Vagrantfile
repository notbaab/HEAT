# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  config.vm.box = "generic/ubuntu1804"
  config.vm.box_check_update = true

  config.vm.synced_folder ".", "/HEAT"
  config.vm.provision "shell", inline: <<-SHELL
    apt-get update -y
    apt-get install -y cmake libsdl2-dev libsdl2-image-dev fish
  SHELL
end
