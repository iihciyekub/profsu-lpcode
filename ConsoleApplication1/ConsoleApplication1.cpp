#include <iostream>
#include <stdlib.h>
#include<ilcplex/ilocplex.h>
ILOSTLBEGIN

int main()
{
#pragma region(S) init set
	IloEnv env;
	IloModel model(env);
	IloNumVarArray call_x(env), put_x(env);
	IloNumVarArray put_isBid(env), put_isAsk(env);

	for (size_t i = 0; i < 7; i++)
	{
		call_x.add(IloNumVar(env, -0, IloInfinity, ILOFLOAT));
		put_x.add(IloNumVar(env, -0, IloInfinity, ILOFLOAT));

	}

	IloNumVar y(env, 0, IloInfinity, ILOFLOAT), z(env, 0, IloInfinity, ILOFLOAT);


	IloCplex cpl(model);


	int num = 7;
	IloIntArray k(env, num);
	IloNumArray call_Bid(env, num),call_Ask(env, num),call_Expected(env, num);
	IloNumArray put_Bid(env, num),put_Ask(env, num),put_Expected(env, num);

	IloNum S_0 = 760.48;
	IloNum S_T = 765.32;

	IloNum cash = 100;
	IloNum theta = 0.9;
	IloNum r = 0.0037;
	IloNum mu_by_year = 765.32;
	IloNum rate_by_year = 0.0037;
	IloNum SD_by_year = 0.179;
	IloNum Dividends = 1.59;
	IloNum horizon = 23;
	IloNum Day_of_year = 360;


#pragma region(S) input data

	FILE* input_para;
	char tempstr[256];
	double tempnum;
	auto filepath = "./data.txt";
	while (fopen_s(&input_para, filepath, "r") != 0)
	{
		cout << "当前目录中没有找到 Parameters.txt..... \n放置文件后,请按回车继续" << endl;
		system("pause");
	}
	// skip the first raw
	fgets(tempstr, 255, input_para);
	// read call option data
	for (int i = 0; i < 7; i++)
	{
		fscanf_s(input_para, "%s", &tempstr,10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &call_Bid[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &call_Ask[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &call_Expected[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &tempnum);
		//env.out() << k[i] << "," << call_Bid[i] << "," << call_Ask[i] << "," << call_Expected[i] << endl;

	}


	// read put option data
	for (size_t i = 0; i < 7; i++)
	{
		fscanf_s(input_para, "%s", &tempstr,10);
		fscanf_s(input_para, "%d", &k[i]);
		fscanf_s(input_para, "%lf", &put_Bid[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &put_Ask[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &put_Expected[i]);
		fscanf_s(input_para, "%lf", &tempnum);
		fscanf_s(input_para, "%lf", &tempnum);
		//env.out() << k[i] <<"," << put_Bid[i]<< "," << put_Ask[i] << "," << put_Expected[i] << endl;

	}
	//system("pause");

	std::fclose(input_para);
	 
#pragma endregion(E)


#pragma region(S) Obj Exp here:

	IloNumExpr sum_Exp(env);
	for (int i = 0; i < 7; i++) {
		sum_Exp += (call_x[i] * call_Expected[i] * 1 + put_x[i] * put_Expected[i]);
	}

	// Obj
	model.add(IloMaximize(env, sum_Exp + y * 765.32 + z * (1 + r)));

#pragma endregion








#pragma region(S) constraints Exp here:


	IloNumExpr sum_Ask(env);
	for (int i = 0; i < 7; i++) {
		sum_Ask += (call_x[i] * call_Ask[i] * 1 + put_x[i] * put_Ask[i]);
	}

	IloNumExpr sum_Bid(env);
	for (int i = 0; i < 7; i++) {
		sum_Bid += (call_x[i] * call_Bid[i] * 1 + put_x[i] * put_Bid[i]);
	}

	// contraints (17)
	IloAdd(model, sum_Ask + y * S_0 + z <= cash);



	IloNumExpr  V_fun_exp(env);
	for (int i = 0; i < 7; i++) {
		V_fun_exp += (put_x[i] * IloMax(0, (k[i] - S_T)) + call_x[i] * IloMax(0, (S_T - k[i])));
	}
	V_fun_exp += z * (1 + r) ;
	
	
	// constraints (18)
	IloAdd(model, V_fun_exp + y * 0 >= cash* theta);
	
	
	// constraints (19)
	IloAdd(model, V_fun_exp + y * k[0] >= V_fun_exp + y * 0);	
	
	for (int i = 1; i < 6; i++) {
		IloAdd(model, V_fun_exp + y* k[i + 1] >= V_fun_exp + y * k[i]);
	}
	//IloAdd(model, V_fun_exp + y *763 >=150);
//
	IloNumExpr  sum_callx(env);
	for (int i = 0; i < 7; i++) {
		sum_callx += call_x[i];
	}
	IloAdd(model, y + sum_callx >= 0);

#pragma endregion



	cpl.solve();


#pragma region (S) output optimal solution  

	for (size_t i = 0; i < 7; i++)
	{
		printf("%s %d %s %lf %s", "Call-",k[i], ":\t",cpl.getValue(call_x[i]), "\n");
	}

	for (size_t i = 0; i < 7; i++)
	{
		printf("%s %d %s %lf %s", "Put-", k[i], ":\t", cpl.getValue(put_x[i]), "\n");
	}

	printf("%s %lf %s","y:", cpl.getValue(y),"\n");
	printf("%s %lf %s", "z:", cpl.getValue(z), "\n");
	printf("%s %lf %s", "Obj:", cpl.getObjValue(), "\n");

#pragma endregion

	system("pause");
}
