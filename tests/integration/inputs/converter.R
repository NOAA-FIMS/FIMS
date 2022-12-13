library(jsonlite)
setwd("/Users/mattadmin/FIMS-Testing/IntegrationTest/FIMS/tests/integration/inputs")

for(i in 0:12){
  for( j in 1:160){
    dir<-paste0("C",i)
    inp<-paste0(paste0(paste0(dir,"/OM"),j),".RData")
    load(inp)
    #out<-paste0(paste0(paste0(dir,"/OM"),j),".json")
    write_json(om_output,paste0(paste0(paste0(dir,"/om_output"),j),".json"))
    write_json(om_input,paste0(paste0(paste0(dir,"/om_input"),j),".json"))
    write_json(em_input,paste0(paste0(paste0(dir,"/em_input"),j),".json"))
    rm(om_input, em_input,om_output)
  }
}
