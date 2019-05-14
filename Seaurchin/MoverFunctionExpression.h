#pragma once

class MoverFunctionExpressionVariables
{
public:
	MoverFunctionExpressionVariables()
		: Begin(0)
		, End(0)
		, Current(0)
		, Progress(0)
	{}

public:
	double Begin;
	double End;
	double Diff;
	double Current;
	double Progress;
};

class MoverFunctionExpression
{
public:
	MoverFunctionExpression() {};
	virtual ~MoverFunctionExpression() {};
	virtual double Execute(const MoverFunctionExpressionVariables& var) const = 0;
};

typedef std::shared_ptr<MoverFunctionExpression> MoverFunctionExpressionSharedPtr;
typedef std::shared_ptr<MoverFunctionExpressionVariables> MoverFunctionExpressionVariablesSharedPtr;


class MoverFunctionExpressionManager
{
private:
	MoverFunctionExpressionManager() = default;
	~MoverFunctionExpressionManager() = default;

	static MoverFunctionExpressionManager* inst;

	std::map<std::string, MoverFunctionExpressionSharedPtr> list;

public:
	MoverFunctionExpressionManager(const MoverFunctionExpressionManager&) = delete;
	MoverFunctionExpressionManager& operator=(const MoverFunctionExpressionManager&) = delete;
	MoverFunctionExpressionManager(MoverFunctionExpressionManager&&) = delete;
	MoverFunctionExpressionManager& operator=(const MoverFunctionExpressionManager&&) = delete;

	static MoverFunctionExpressionManager& GetInstance() { return *inst; }
	static bool Initialize();
	static bool Finalize();

	static bool Register(const std::string& key, const std::string& expression);
	bool Register(const std::string& key, MoverFunctionExpression* pFunction);
	bool Register(const std::string& key, const MoverFunctionExpressionSharedPtr& pFunction);

	static bool IsRegistered(const std::string& key);

	bool Find(const std::string& key, MoverFunctionExpressionSharedPtr& pFunction) const;
};
