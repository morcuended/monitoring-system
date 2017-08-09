####################
# tested on python 3
####################
import mysql.connector
from mysql.connector import errorcode


# Define database parameters
myTable="Test_Camera1U_FFCoeff"
myTable_Set="Test_Camera1U_FFCoeff_Set"


config = {
  'user': 'dan',
  'password':'pass',
  # 'host': '141.34.130.13',
  'database':'mydb',
  'raise_on_warnings': True,
}

try:
  cnx = mysql.connector.connect(**config)
  print("You are connected \n")
except mysql.connector.Error as err:
  if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
    print("Something is wrong with your user name or password \n")
  elif err.errno == errorcode.ER_BAD_DB_ERROR:
    print("Database does not exist \n")
  else:
    print(err)
else:
  cursor=cnx.cursor()
#################################################
cursor.execute("SHOW TABLES;")
a=cursor.fetchall()
table_list = map(lambda x:x[0],a)

if myTable in table_list:
    print("Table %s already exists \n"%myTable)
else:
    cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff (
    entry int NOT NULL auto_increment PRIMARY KEY,
    SetNum int NOT NULL,
    RunNumber int NOT NULL,
    RunStarts DATETIME,
    pixel int NOT NULL,
    HiIllumination double,
    HiIlluminationRMS double,
    LoIllumination double,
    LoIlluminationRMS double,
    HiCoeff double,
    HiCoeffRMS double,
    LoCoeff double,
    LoCoefdRMS double,
    TelescopeId int,
    index(SetNum)); """); print("Table %s was created \n"%myTable)
cnx.commit()

#####################################################
if myTable_Set in table_list:
        print("Table %s already exists \n"%myTable_Set)
else:
	cursor.execute("""CREATE TABLE Test_Camera1U_FFCoeff_Set (
	SetNum int NOT NULL auto_increment PRIMARY KEY,
	TelescopeId int NOT NULL,
	WhenEntered TIMESTAMP,
	index(TelescopeId)); """); print("Table %s was created \n"%myTable_Set)
	cnx.commit()
cnx.close()
cursor.close()
