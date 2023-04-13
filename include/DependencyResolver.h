#pragma once

// See SPID for documentation

template <typename Value, typename Comparator = std::less<Value>>
class DependencyResolver
{
public:
	DependencyResolver() = default;
	DependencyResolver(const std::vector<Value>& values) :
		comparator(std::move(Comparator()))
	{
		for (const auto& value : values) {
			nodes.try_emplace(value, new Node(value, comparator));
		}
	}
	~DependencyResolver()
	{
		for (const auto& pair : nodes) {
			delete pair.second;
		}
	}

	void AddIsolated(const Value& value)
	{
		if (!nodes.contains(value)) {
			nodes.try_emplace(value, new Node(value, comparator));
		}
	}
	void AddDependency(const Value& lhs, const Value& rhs)
	{
		try {
			addDependency(lhs, rhs);
		} catch (SelfReferenceDependencyException& e) {
			buffered_logger::warn("\tINFO - {} is referencing itself", describe(e.current));
		} catch (CyclicDependencyException& e) {
			std::ostringstream os;
			os << e.path.top();
			auto path = e.path;
			path.pop();
			while (!path.empty()) {
				os << " -> " << path.top();
				path.pop();
			}
			buffered_logger::warn("\tINFO - {} and {} depend on each other. Distribution might not work as expected.\n\t\t\t\t\tFull path: {}", describe(e.first), describe(e.second), os.str());
		} catch (...) {
			// we'll ignore other exceptions
		}
	}
	[[nodiscard]] std::vector<Value> Resolve() const
	{
		std::vector<Value> result;

		/// A vector of nodes that are ordered in a way that would make resolution the most efficient
		///	by reducing number of lookups for all nodes to resolved the graph.
		std::vector<Node*> orderedNodes;

		std::ranges::transform(nodes, std::back_inserter(orderedNodes), [](const auto& pair) {
			return pair.second;
		});
		// Sort nodes in correct order of processing.
		std::sort(orderedNodes.begin(), orderedNodes.end(), node_less());

		for (const auto& node : orderedNodes) {
			node->isResolved = false;
		}

		for (const auto& node : orderedNodes) {
			resolveNode(node, result);
		}

		return result;
	}

	struct SelfReferenceDependencyException : std::exception
	{
		SelfReferenceDependencyException(const Value& current) :
			current(current)
		{}

		const Value& current;
	};

	struct CyclicDependencyException : std::exception
	{
		CyclicDependencyException(Value first, Value second, std::stack<Value> path) :
			first(std::move(first)),
			second(std::move(second)),
			path(std::move(path))
		{}

		const Value             first;
		const Value             second;
		const std::stack<Value> path;
	};

	struct SuperfluousDependencyException : std::exception
	{
		SuperfluousDependencyException(Value current, Value superfluous, std::stack<Value> path) :
			current(std::move(current)),
			superfluous(std::move(superfluous)),
			path(std::move(path))
		{}

		const Value             current;
		const Value             superfluous;
		const std::stack<Value> path;
	};

private:
	struct Node
	{
		const Value value;

		/// Comparator functor that is used to compare Value of two Nodes.
		const Comparator& comparator;

		/// A list of all other nodes that current node depends on.
		///
		/// <p>
		///	<b>Use dependsOn() method to determine whether current node depends on another</b>.
		///	</p>
		Set<Node*> dependencies{};

		/// Flag that is used by DependencyResolver during resolution process
		///	to detect whether given Node was already resolved.
		///
		///	<p>
		///	This helps avoid unnecessary iterations over the same nodes,
		///	which might occur when it is a part of several dependencies lists.
		///	</p>
		bool isResolved;

		Node(Value value, const Comparator& comparator) :
			value(std::move(value)), comparator(comparator), isResolved(false) {}
		~Node() = default;

		bool dependsOn(Node* const node, std::stack<Value>& path) const
		{
			if (dependencies.empty()) {
				return false;
			}

			if (dependencies.contains(node)) {
				path.push(node->value);
				path.push(this->value);
				return true;
			}

			if (auto nextNode = std::find_if(dependencies.begin(), dependencies.end(), [&](const auto& dependency) { return dependency->dependsOn(node, path); }); nextNode != dependencies.end()) {
				path.push(this->value);
				return true;
			}

			return false;
		}

		/// May throw an exception if Directed Acyclic Graph rules will be violated.
		void addDependency(Node* node)
		{
			assert(node != nullptr);

			if (this == node) {
				throw SelfReferenceDependencyException(this->value);
			}

			std::stack<Value> cyclicPath;

			if (node->dependsOn(this, cyclicPath)) {
				cyclicPath.push(this->value);
				throw CyclicDependencyException(this->value, node->value, cyclicPath);
			}

			std::stack<Value> superfluousPath;

			if (this->dependsOn(node, superfluousPath)) {
				throw SuperfluousDependencyException(this->value, node->value, superfluousPath);
			}

			if (!dependencies.emplace(node).second) {
				// this->dependsOn(node) should always detect when duplicated dependency is added, but just to be safe.. :)
				throw SuperfluousDependencyException(this->value, node->value, {});
			}
		}

		bool operator==(const Node& other) const
		{
			return this->value == other.value;
		}

		bool operator<(const Node& other) const
		{
			if (this->dependencies.size() < other.dependencies.size()) {
				return true;
			}
			if (this->dependencies.size() == other.dependencies.size()) {
				return comparator(this->value, other.value);
			}

			return false;
		}
	};

	struct node_less
	{
		bool operator()(const Node* lhs, const Node* rhs) const
		{
			return *lhs < *rhs;
		}
	};

	void addDependency(const Value& parent, const Value& dependency)
	{
		Node* parentNode;
		Node* dependencyNode;

		if (const auto it = nodes.find(parent); it != nodes.end()) {
			parentNode = it->second;
		} else {
			parentNode = new Node(parent, comparator);
			nodes.try_emplace(parent, parentNode);
		}

		if (const auto it = nodes.find(dependency); it != nodes.end()) {
			dependencyNode = it->second;
		} else {
			dependencyNode = new Node(dependency, comparator);
			nodes.try_emplace(dependency, dependencyNode);
		}

		if (parentNode && dependencyNode) {
			parentNode->addDependency(dependencyNode);
		}
	}
	void resolveNode(Node* const node, std::vector<Value>& result) const
	{
		if (node->isResolved) {
			return;
		}
		for (const auto& dependency : node->dependencies) {
			resolveNode(dependency, result);
		}
		result.push_back(node->value);
		node->isResolved = true;
	}

	// members
	const Comparator  comparator;
	Map<Value, Node*> nodes{};
};
